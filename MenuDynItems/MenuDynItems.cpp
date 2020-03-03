#include <stdio.h>

#include <Application.h>
#include <Window.h>
#include <View.h>
#include <Rect.h>
#include <Menu.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <MenuBar.h>
#include <String.h>

void BuildMenu(BMenu *dst, int level)
{
	char buf[64];
	int cnt = 10;
	BMenu *subMenu;
	if (level == 0) {
		dst->AddItem(new BMenuItem("Menu item", new BMessage('item')));
		subMenu = new BMenu("Long menu");
		dst->AddItem(subMenu);
		for (int i = 0; i < 200; ++i) {
				sprintf(buf, "Item %d", i);
				subMenu->AddItem(new BMenuItem(buf, new BMessage('item')));
		}
		cnt = 16;
	}

	for (int i = 0; i < cnt; ++i) {
		if (level < 3) {
			sprintf(buf, "%d", i);
			subMenu = new BMenu(buf);
			BuildMenu(subMenu, level + 1);
			dst->AddItem(subMenu);
		} else {
			dst->AddItem(new BMenuItem("Item", new BMessage('item')));
		}
	}
}

class TestMenuBar: public BMenuBar
{
public:
	TestMenuBar(): BMenuBar(BRect(0, 0, 32, 32), "menu", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_ITEMS_IN_ROW, true)
	{
		BuildMenu(this, 0);
	}

	bool UpdateDynItems(int32 modifiers)
	{
		BMenu *menu = ItemAt(2)->Submenu();
		if (menu == NULL || menu->Window() == NULL || !menu->LockLooper())
			return false;

		int32 count = menu->CountItems() - 1;
			// don't update the last item
		for (int32 index = 0; index < count; index++) {
			BString label;
			label << index;
			const char* origText = label.String();
			BMenuItem* item = menu->ItemAt(index);
			if ((modifiers & B_SHIFT_KEY) != 0) {
				BString buf(origText);
				buf << " (Shift)";
				item->SetLabel(buf.String());
				if (index < 10)
					item->SetShortcut((char)(index + 48), B_SHIFT_KEY);
			} else
				item->SetLabel(origText);

			if (index < 10)
				item->SetShortcut((char)(index + 48), modifiers);

			menu->RemoveItem(item);
			menu->AddItem(item, index);
		}
		menu->UnlockLooper();
		return true;
	}

	void MessageReceived(BMessage *msg)
	{
		switch (msg->what) {
			case B_MODIFIERS_CHANGED: {
				int32 modifiers = 0;
				msg->FindInt32("modifiers", &modifiers);
				UpdateDynItems(modifiers);
				break;
			}
		}
		BMenuBar::MessageReceived(msg);
	}

private:
};

class TestWindow: public BWindow
{
private:
	TestMenuBar *fMenu;
public:
	TestWindow(BRect frame): BWindow(frame, "TestApp", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS)
	{
		fMenu = new TestMenuBar();
		this->AddChild(fMenu);
	}

	~TestWindow()
	{
	}

	void MenusBeginning()
	{
		fMenu->SetEventMask(fMenu->EventMask() | B_KEYBOARD_EVENTS);
		fMenu->UpdateDynItems(modifiers());
	}

	void MenusEnded()
	{
		fMenu->SetEventMask(fMenu->EventMask() & ~B_KEYBOARD_EVENTS);
	}

};

class TestApplication: public BApplication
{
public:
	TestApplication(): BApplication("application/x-vnd.test.app")
	{
	}

	void ReadyToRun() {
		BWindow *wnd = new TestWindow(BRect(32, 32, 512, 256));
		wnd->SetFlags(wnd->Flags() | B_QUIT_ON_WINDOW_CLOSE);
		wnd->Show();
	}
};


int main()
{
	(new TestApplication())->Run();
	return 0;
}

/*
gcc -lbe -lstdc++ MenuDyn.cpp -o MenuDyn
MenuDyn
*/
