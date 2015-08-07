#include "stdafx.h"

// �� � ���� �� ������ ���� 
FIXME(��������� ��� � ����������)
#include <cegui/CEGUI.h>

#include "vis_settings_panel_impl.h"

using namespace CEGUI;

const  String combo_name     ( "selModeBox"		);
const  String btn_exit_name  ( "btnExit"		);
const  String main_menu_name ( "mainMenuBar"    );
const  String tb_name		 ( "tbLight"        );

 void subscribeEvent(const String& widget, const String& event, const Event::Subscriber& method)
{
    GUIContext& context = System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();

    if (root->isChild(widget))
    {
        Window* window = root->getChild(widget);
        window->subscribeEvent(event, method);
    }
}

bool isCheckboxSelected(const CEGUI::String& checkbox)
{
    GUIContext& context = System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    // Check
    if (root->isChild(checkbox))
    {
        ToggleButton* button = static_cast<ToggleButton*>(root->getChild(checkbox));
        return button->isSelected();
    }
    return false;
}

std::string getEditboxText(const CEGUI::String& editbox)
{
	GUIContext& context = System::getSingleton().getDefaultGUIContext();
	CEGUI::Window* root = context.getRootWindow();
	// Check
	if (root->isChild(editbox))
	{
		Editbox* button = static_cast<Editbox*>(root->getChild(editbox));
		return std::string(button->getText().c_str());
	}
	return "";
}

vis_settings_panel_impl::vis_settings_panel_impl(  const app::zones_t &zones )
{
    GUIContext& context = System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    
	WindowManager& winMgr = WindowManager::getSingleton();

    FrameWindow* demoWindow = static_cast<FrameWindow*>(
        CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/FrameWindow", "DemoWindow") );
    demoWindow->setPosition(UVector2(cegui_reldim(0.01f), cegui_reldim(0.5f)));
    //demoWindow->setSize(USize(cegui_reldim(0.3f), cegui_reldim(0.3f)));
    demoWindow->setMinSize(USize(cegui_reldim(0.1f), cegui_reldim(0.1f)));
    demoWindow->setText( "Choose Zone" );

    PushButton* demoButtonOK = static_cast<PushButton*>(
        WindowManager::getSingleton().createWindow("TaharezLook/Button", "DemoButtonOK") );
    demoButtonOK->setPosition(UVector2(cegui_reldim(0.3f), cegui_reldim(0.75f)));
    demoButtonOK->setSize( USize(cegui_reldim(0.4f), cegui_reldim(0.15f)) );
    demoButtonOK->setText( "OK" );

	auto fn_exit = [=](const CEGUI::EventArgs& args)->bool 
        {
			CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
			CEGUI::Window* root = context.getRootWindow();
			root->getChild("DemoWindow")->setVisible(false);
            return true;

        };

    demoWindow->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked,Event::Subscriber(fn_exit));
	demoButtonOK->subscribeEvent( CEGUI::PushButton::EventClicked,Event::Subscriber(fn_exit));



	PushButton* btnExit = static_cast<PushButton*>(
        WindowManager::getSingleton().createWindow("TaharezLook/Button", btn_exit_name) );

    root->addChild(btnExit);
    btnExit->setPosition(UVector2(cegui_reldim(0.95f), cegui_reldim(0.95f)));
    btnExit->setSize( USize(cegui_reldim(0.04f), cegui_reldim(0.02f)) );
    btnExit->setText( "Exit" );
    
    btnExit->subscribeEvent(PushButton::EventClicked, 
        Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
        {

            exit_app_signal_(); 
            return true;

        })
        ); 
    

    Combobox* cbbo = static_cast<Combobox*>( CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Combobox", combo_name));
    cbbo->setPosition(UVector2(cegui_reldim(0.15f), cegui_reldim( 0.1f)));
    //cbbo->setSize(USize(cegui_reldim(0.66f), cegui_reldim( 0.33f)));
    demoWindow->addChild(cbbo);
	demoWindow->addChild( demoButtonOK );
    root->addChild( demoWindow );

    demoWindow->setVisible(false);

    ListboxTextItem* itm;
    CEGUI::Win32StringTranscoder stc;
    const CEGUI::Image* sel_img = &ImageManager::getSingleton().get("TaharezLook/MultiListSelectionBrush");

    for (auto it = zones.begin(); it!=zones.end();++it)
    {
        itm = new ListboxTextItem(stc.stringFromStdWString(it->second), it->first);
        itm->setSelectionBrushImage(sel_img);
        cbbo->addItem(itm);
    }
    
    cbbo->subscribeEvent(Combobox::EventListSelectionAccepted, 
        Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
            {
                Combobox* combo = static_cast<Combobox*>(static_cast<const WindowEventArgs&>(args).window->getRootWindow()->getChild("DemoWindow/" + combo_name));
                
                ListboxItem* item = combo->findItemWithText(combo->getText(), 0);
                if (item)
                {
                    zone_changed_signal_( item->getID()); 
                }

                return true;
    
            })
        ); 

    cbbo->setReadOnly(true);
    cbbo->setSortingEnabled(false);

    //cbbo->handleUpdatedListItemData();

	CEGUI::ToggleButton*    checkbox = static_cast<ToggleButton*>( CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Checkbox", tb_name));
	checkbox->setSelected(true);
	checkbox->setText( "Lights" );
	demoWindow->addChild(checkbox);
	
	root->addChild(winMgr.loadLayoutFromFile("vis_settings.layout"));
	subscribeEvent("FrameWindow/LightParams/chkLights", ToggleButton::EventSelectStateChanged,
		Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
        {
            
			bool wrap = isCheckboxSelected("FrameWindow/LightParams/chkLights");
			set_lights_signal_(wrap); 
            return true;

        }));

	subscribeEvent("FrameWindow/LightParams/edtGrassMap", CEGUI::Editbox::EventTextAccepted,
		Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
	{

		std::string param = getEditboxText("FrameWindow/LightParams/edtGrassMap");
		set_map_signal_(boost::lexical_cast<float>(param.empty()?"0":param)); 
		return true;

	}));

	subscribeEvent("FrameWindow/LightParams/btnApply", PushButton::EventClicked,
		Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
	{

		std::string param = getEditboxText("FrameWindow/LightParams/edtGrassMap");
		set_map_signal_(boost::lexical_cast<float>(param.empty()?"0":param)); 
		return true;

	}));

 
	//CEGUI::Menubar* menuBar = dynamic_cast<CEGUI::Menubar*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/Menubar", main_menu_name));
	//if(menuBar)
	//{     
	//	init_menu_bar(menuBar);
	//	root->addChild(menuBar);
	//}
}

vis_settings_panel_impl::~vis_settings_panel_impl()
{
}

void vis_settings_panel_impl::set_visible(bool visible)
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    root->getChild(combo_name)->setVisible(visible);
    root->getChild(btn_exit_name)->setVisible(visible);
    //root->getChild(main_menu_name)->setVisible(visible);

}

bool vis_settings_panel_impl::visible()
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    return root->getChild(btn_exit_name)->isVisible() || root->getChild(combo_name)->isVisible();
}

void vis_settings_panel_impl::set_light(bool on)
{
    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    if (root->isChild("FrameWindow/LightParams/chkLights"))
    {
        ToggleButton* button = static_cast<ToggleButton*>(root->getChild("FrameWindow/LightParams/chkLights"));
        return button->setSelected(on);
    }
}

void vis_settings_panel_impl::init_menu_bar(CEGUI::Menubar* menuBar)
{
	CEGUI::String skin = menuBar->getType();
	skin = skin.substr(0, skin.find_first_of('/'));
	CEGUI::String menuItemMapping = skin + "/MenuItem";
	CEGUI::String popupMenuMapping = skin + "/PopupMenu";

	CEGUI::WindowManager& windowManager = CEGUI::WindowManager::getSingleton(); 
	CEGUI::MenuItem* fileMenuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "FileMenuItem"));
	fileMenuItem->setText("File");
	menuBar->addChild(fileMenuItem);

	CEGUI::PopupMenu* filePopupMenu = static_cast<CEGUI::PopupMenu*>(windowManager.createWindow(popupMenuMapping, "FilePopupMenu"));
	fileMenuItem->addChild(filePopupMenu);

	CEGUI::MenuItem* menuItem;
	menuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "FileTestMenuItem1"));
	menuItem->setText("Open");
	filePopupMenu->addItem(menuItem);

	menuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "FileTestMenuItem2"));
	menuItem->setText("Save");
	filePopupMenu->addItem(menuItem);

	menuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "FileTestMenuItem3"));
	menuItem->setText("Exit");
	filePopupMenu->addItem(menuItem);

	menuItem->subscribeEvent(MenuItem::EventClicked, 
		Event::Subscriber([=](const CEGUI::EventArgs& args)->bool 
		{

			exit_app_signal_(); 
			return true;

		})
		); 

	CEGUI::MenuItem* viewMenuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "ViewMenuItem"));
	viewMenuItem->setText("View");
	menuBar->addChild(viewMenuItem);

	CEGUI::PopupMenu* viewPopupMenu = static_cast<CEGUI::PopupMenu*>(windowManager.createWindow(popupMenuMapping, "ViewPopupMenu"));
	viewMenuItem->addChild(viewPopupMenu);

	menuItem = static_cast<CEGUI::MenuItem*>(windowManager.createWindow(menuItemMapping, "ViewTestMenuItem1"));
	menuItem->setText("Lights");
	viewPopupMenu->addItem(menuItem);
}

namespace app
{
    vis_settings_panel_ptr create_vis_settings_panel(const app::zones_t &zones )
    {
        return boost::make_shared<vis_settings_panel_impl>(  zones );
    }
}