/*
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 * Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 *
 * Portions Copyright (C) 2000 TrollTech AS.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kcmstyle.h"

#include <config-X11.h>

#include "styleconfdialog.h"
#include "ui_stylepreview.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kstyle.h>
#include <kstandarddirs.h>

#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtGui/QLabel>
#include <QtGui/QPixmapCache>
#include <QtGui/QStyleFactory>
#include <QtDBus/QtDBus>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

#include "../krdb/krdb.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

// X11 namespace cleanup
#undef Below
#undef KeyPress
#undef KeyRelease


/**** DLL Interface for kcontrol ****/

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KLibLoader>

K_PLUGIN_FACTORY(KCMStyleFactory, registerPlugin<KCMStyle>();)
K_EXPORT_PLUGIN(KCMStyleFactory("kcmstyle"))


extern "C"
{
    KDE_EXPORT void kcminit_style()
    {
        uint flags = KRdbExportQtSettings | KRdbExportQtColors | KRdbExportXftSettings;
        KConfig _config( "kcmdisplayrc", KConfig::CascadeConfig  );
        KConfigGroup config(&_config, "X11");

        // This key is written by the "colors" module.
        bool exportKDEColors = config.readEntry("exportKDEColors", true);
        if (exportKDEColors)
            flags |= KRdbExportColors;
        runRdb( flags );

        // Write some Qt root property.
#ifdef Q_WS_X11
#ifndef __osf__      // this crashes under Tru64 randomly -- will fix later
        QByteArray properties;
        QDataStream d(&properties, QIODevice::WriteOnly);
        d.setVersion( 3 );      // Qt2 apps need this.
        d << kapp->palette() << KGlobalSettings::generalFont();
        Atom a = XInternAtom(QX11Info::display(), "_QT_DESKTOP_PROPERTIES", false);

        // do it for all root windows - multihead support
        int screen_count = ScreenCount(QX11Info::display());
        for (int i = 0; i < screen_count; i++)
            XChangeProperty(QX11Info::display(), RootWindow(QX11Info::display(), i),
                            a, a, 8, PropModeReplace,
                            (unsigned char*) properties.data(), properties.size());
#endif
#endif
    }
}

class StylePreview : public QWidget, public Ui::StylePreview
{
public:
    StylePreview(QWidget *parent = 0)
    : QWidget(parent)
    {
        setupUi(this);

        // Ensure that the user can't toy with the child widgets.
        // Method borrowed from Qt's qtconfig.
        QList<QWidget*> widgets = findChildren<QWidget*>();
        foreach (QWidget* widget, widgets)
        {
            widget->installEventFilter(this);
            widget->setFocusPolicy(Qt::NoFocus);
        }
    }

    bool eventFilter( QObject* /* obj */, QEvent* ev )
    {
        switch( ev->type() )
        {
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseButtonDblClick:
            case QEvent::MouseMove:
            case QEvent::KeyPress:
            case QEvent::KeyRelease:
            case QEvent::Enter:
            case QEvent::Leave:
            case QEvent::Wheel:
            case QEvent::ContextMenu:
                return true; // ignore
            default:
                break;
        }
        return false;
    }
};


KCMStyle::KCMStyle( QWidget* parent, const QVariantList& )
	: KCModule( KCMStyleFactory::componentData(), parent ), appliedStyle(NULL)
{
    setQuickHelp( i18n("<h1>Style</h1>"
			"This module allows you to modify the visual appearance "
			"of user interface elements, such as the widget style "
			"and effects."));

	m_bEffectsDirty = false;
	m_bStyleDirty= false;
	m_bToolbarsDirty = false;

	KGlobal::dirs()->addResourceType("themes", "data", "kstyle/themes");

	KAboutData *about =
		new KAboutData( I18N_NOOP("kcmstyle"), 0,
						ki18n("KDE Style Module"),
						0, KLocalizedString(), KAboutData::License_GPL,
						ki18n("(c) 2002 Karol Szwed, Daniel Molkentin"));

	about->addAuthor(ki18n("Karol Szwed"), KLocalizedString(), "gallium@kde.org");
	about->addAuthor(ki18n("Daniel Molkentin"), KLocalizedString(), "molkentin@kde.org");
	about->addAuthor(ki18n("Ralf Nolden"), KLocalizedString(), "nolden@kde.org");
	setAboutData( about );

	// Setup pages and mainLayout
	mainLayout = new QVBoxLayout( this );
	mainLayout->setMargin(0);

	tabWidget  = new QTabWidget( this );
	mainLayout->addWidget( tabWidget );

	page1 = new QWidget;
	page1Layout = new QVBoxLayout( page1 );
	page2 = new QWidget;
	page2Layout = new QVBoxLayout( page2 );
	page3 = new QWidget;
	page3Layout = new QVBoxLayout( page3 );

	// Add Page1 (Style)
	// -----------------
	gbWidgetStyle = new QGroupBox( i18n("Widget Style"), page1 );
	QVBoxLayout *widgetLayout = new QVBoxLayout(gbWidgetStyle);

	gbWidgetStyleLayout = new QVBoxLayout( );
        widgetLayout->addLayout( gbWidgetStyleLayout );
	gbWidgetStyleLayout->setAlignment( Qt::AlignTop );
	hbLayout = new QHBoxLayout( );
	hbLayout->setObjectName( "hbLayout" );

	cbStyle = new KComboBox( gbWidgetStyle );
        cbStyle->setObjectName( "cbStyle" );
	cbStyle->setEditable( false );
	hbLayout->addWidget( cbStyle );

	pbConfigStyle = new QPushButton( i18n("Con&figure..."), gbWidgetStyle );
	pbConfigStyle->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Minimum );
	pbConfigStyle->setEnabled( false );
	hbLayout->addWidget( pbConfigStyle );

	gbWidgetStyleLayout->addLayout( hbLayout );

	lblStyleDesc = new QLabel( gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( lblStyleDesc );

	cbIconsOnButtons = new QCheckBox( i18n("Sho&w icons on buttons"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbIconsOnButtons );
	cbEnableTooltips = new QCheckBox( i18n("E&nable tooltips"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbEnableTooltips );
	cbTearOffHandles = new QCheckBox( i18n("Show tear-off handles in &popup menus"), gbWidgetStyle );
	gbWidgetStyleLayout->addWidget( cbTearOffHandles );
	cbTearOffHandles->hide(); // reenable when the corresponding Qt method is virtual and properly reimplemented

	QGroupBox *gbPreview = new QGroupBox( i18n( "Preview" ), page1 );
	QVBoxLayout *previewLayout = new QVBoxLayout(gbPreview);
	previewLayout->setMargin( 0 );
	stylePreview = new StylePreview( gbPreview );
	gbPreview->layout()->addWidget( stylePreview );

	page1Layout->addWidget( gbWidgetStyle );
	page1Layout->addWidget( gbPreview );
	page1Layout->addStretch();

	// Connect all required stuff
	connect( cbStyle, SIGNAL(activated(int)), this, SLOT(styleChanged()) );
	connect( cbStyle, SIGNAL(activated(int)), this, SLOT(updateConfigButton()));
	connect( pbConfigStyle, SIGNAL(clicked()), this, SLOT(styleSpecificConfig()));

	// Add Page2 (Effects)
	// -------------------
	cbEnableEffects = new QCheckBox( i18n("&Enable GUI effects"), page2 );
	containerFrame = new QFrame( page2 );
	containerFrame->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
	containerLayout = new QGridLayout( containerFrame );

	comboComboEffect = new QComboBox( containerFrame );
	comboComboEffect->setEditable( false );
	comboComboEffect->addItem( i18n("Disable") );
	comboComboEffect->addItem( i18n("Animate") );
	lblComboEffect = new QLabel( i18n("Combobo&x effect:"), containerFrame );
	lblComboEffect->setBuddy( comboComboEffect );
	containerLayout->addWidget( lblComboEffect, 0, 0 );
	containerLayout->addWidget( comboComboEffect, 0, 1 );

	comboTooltipEffect = new QComboBox( containerFrame );
	comboTooltipEffect->setEditable( false );
	comboTooltipEffect->addItem( i18n("Disable") );
	comboTooltipEffect->addItem( i18n("Animate") );
	comboTooltipEffect->addItem( i18n("Fade") );
	lblTooltipEffect = new QLabel( i18n("&Tool tip effect:"), containerFrame );
	lblTooltipEffect->setBuddy( comboTooltipEffect );
	containerLayout->addWidget( lblTooltipEffect, 1, 0 );
	containerLayout->addWidget( comboTooltipEffect, 1, 1 );

	comboMenuEffect = new QComboBox( containerFrame );
	comboMenuEffect->setEditable( false );
	comboMenuEffect->addItem( i18n("Disable") );
	comboMenuEffect->addItem( i18n("Animate") );
	comboMenuEffect->addItem( i18n("Fade") );
	comboMenuEffect->addItem( i18n("Make Translucent") );
	lblMenuEffect = new QLabel( i18n("&Menu effect:"), containerFrame );
	lblMenuEffect->setBuddy( comboMenuEffect );
	containerLayout->addWidget( lblMenuEffect, 2, 0 );
	containerLayout->addWidget( comboMenuEffect, 2, 1 );

	comboMenuHandle = new QComboBox( containerFrame );
	comboMenuHandle->setEditable( false );
	comboMenuHandle->addItem( i18n("Disable") );
	comboMenuHandle->addItem( i18n("Application Level") );
//	comboMenuHandle->addItem( i18n("Enable") );
	lblMenuHandle = new QLabel( i18n("Me&nu tear-off handles:"), containerFrame );
	lblMenuHandle->setBuddy( comboMenuHandle );
	containerLayout->addWidget( lblMenuHandle, 3, 0 );
	containerLayout->addWidget( comboMenuHandle, 3, 1 );

	cbMenuShadow = new QCheckBox( i18n("Menu &drop shadow"), containerFrame );
	containerLayout->addWidget( cbMenuShadow, 4, 0 );

	// Push the [label combo] to the left.
	comboSpacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	containerLayout->addItem( comboSpacer, 1, 2 );

	// Separator.
	QFrame* hline = new QFrame ( page2 );
	hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );

	// Now implement the Menu Transparency container.
	menuContainer = new QFrame( page2 );
	menuContainer->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
	menuContainerLayout = new QGridLayout( menuContainer );

	menuPreview = new MenuPreview( menuContainer, /* opacity */ 90, MenuPreview::Blend );

	comboMenuEffectType = new QComboBox( menuContainer );
	comboMenuEffectType->setEditable( false );
	comboMenuEffectType->addItem( i18n("Software Tint") );
	comboMenuEffectType->addItem( i18n("Software Blend") );
#ifdef HAVE_XRENDER
	comboMenuEffectType->addItem( i18n("XRender Blend") );
#endif

	// So much stuffing around for a simple slider..
	sliderBox = new KVBox( menuContainer );
	slOpacity = new QSlider( Qt::Horizontal, sliderBox );
	slOpacity->setMinimum( 0 );
	slOpacity->setMaximum( 100 );
	slOpacity->setPageStep( 5 );
	slOpacity->setTickPosition( QSlider::TicksBelow );
	slOpacity->setTickInterval( 10 );
	KHBox* box1 = new KHBox( sliderBox );
	QLabel* lbl = new QLabel( i18n("0%"), box1 );
	lbl->setAlignment( Qt::AlignLeft );
	lbl = new QLabel( i18n("50%"), box1 );
	lbl->setAlignment( Qt::AlignHCenter );
	lbl = new QLabel( i18n("100%"), box1 );
	lbl->setAlignment( Qt::AlignRight );

	lblMenuEffectType = new QLabel( i18n("Menu trans&lucency type:"), menuContainer );
	lblMenuEffectType->setBuddy( comboMenuEffectType );
	lblMenuEffectType->setAlignment( Qt::AlignBottom | Qt::AlignLeft );
	lblMenuOpacity = new QLabel( i18n("Menu &opacity:"), menuContainer );
	lblMenuOpacity->setBuddy( slOpacity );
	lblMenuOpacity->setAlignment( Qt::AlignBottom | Qt::AlignLeft );

	menuContainerLayout->addWidget( lblMenuEffectType, 0, 0 );
	menuContainerLayout->addWidget( comboMenuEffectType, 1, 0 );
	menuContainerLayout->addWidget( lblMenuOpacity, 2, 0 );
	menuContainerLayout->addWidget( sliderBox, 3, 0 );
	menuContainerLayout->addWidget( menuPreview, 0, 1, 4, 1);

	// Layout page2.
	page2Layout->addWidget( cbEnableEffects );
	page2Layout->addWidget( containerFrame );
	page2Layout->addWidget( hline );
	page2Layout->addWidget( menuContainer );

	QSpacerItem* sp1 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
	page2Layout->addItem( sp1 );

	// Data flow stuff.
	connect( cbEnableEffects,    SIGNAL(toggled(bool)), containerFrame, SLOT(setEnabled(bool)) );
	connect( cbEnableEffects,    SIGNAL(toggled(bool)), this, SLOT(menuEffectChanged(bool)) );
	connect( slOpacity,          SIGNAL(valueChanged(int)),menuPreview, SLOT(setOpacity(int)) );
	connect( comboMenuEffect,    SIGNAL(activated(int)), this, SLOT(menuEffectChanged()) );
	connect( comboMenuEffect,    SIGNAL(highlighted(int)), this, SLOT(menuEffectChanged()) );
	connect( comboMenuEffectType, SIGNAL(activated(int)), this, SLOT(menuEffectTypeChanged()) );
	connect( comboMenuEffectType, SIGNAL(highlighted(int)), this, SLOT(menuEffectTypeChanged()) );

	// Add Page3 (Miscellaneous)
	// -------------------------
	cbHoverButtons = new QCheckBox( i18n("High&light buttons under mouse"), page3 );
	cbTransparentToolbars = new QCheckBox( i18n("Transparent tool&bars when moving"), page3 );

	QWidget * dummy = new QWidget( page3 );

	QHBoxLayout* box2 = new QHBoxLayout( dummy );
	lbl = new QLabel( i18n("Text pos&ition:"), dummy );
	comboToolbarIcons = new QComboBox( dummy );
	comboToolbarIcons->setEditable( false );
	comboToolbarIcons->addItem( i18n("Icons Only") );
	comboToolbarIcons->addItem( i18n("Text Only") );
	comboToolbarIcons->addItem( i18n("Text Alongside Icons") );
	comboToolbarIcons->addItem( i18n("Text Under Icons") );
	lbl->setBuddy( comboToolbarIcons );

	box2->addWidget( lbl );
	box2->addWidget( comboToolbarIcons );
	QSpacerItem* sp2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	box2->addItem( sp2 );

	page3Layout->addWidget( cbHoverButtons );
	page3Layout->addWidget( cbTransparentToolbars );
	page3Layout->addWidget( dummy );

	// Layout page3.
	QSpacerItem* sp3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
	page3Layout->addItem( sp3 );

	// Load settings
	load();

	// Do all the setDirty connections.
	connect(cbStyle, SIGNAL(activated(int)), this, SLOT(setStyleDirty()));
	// Page2
	connect( cbEnableEffects,    SIGNAL(toggled(bool)),   this, SLOT(setEffectsDirty()));
	connect( cbEnableEffects,    SIGNAL(toggled(bool)),   this, SLOT(setStyleDirty()));
	connect( comboTooltipEffect, SIGNAL(activated(int)), this, SLOT(setEffectsDirty()));
	connect( comboComboEffect,   SIGNAL(activated(int)), this, SLOT(setEffectsDirty()));
	connect( comboMenuEffect,    SIGNAL(activated(int)), this, SLOT(setStyleDirty()));
	connect( comboMenuHandle,    SIGNAL(activated(int)), this, SLOT(setStyleDirty()));
	connect( comboMenuEffectType, SIGNAL(activated(int)), this, SLOT(setStyleDirty()));
	connect( slOpacity,          SIGNAL(valueChanged(int)),this, SLOT(setStyleDirty()));
	connect( cbMenuShadow,       SIGNAL(toggled(bool)),   this, SLOT(setStyleDirty()));
	// Page3
	connect( cbHoverButtons,       SIGNAL(toggled(bool)),   this, SLOT(setToolbarsDirty()));
	connect( cbTransparentToolbars, SIGNAL(toggled(bool)),   this, SLOT(setToolbarsDirty()));
	connect( cbEnableTooltips,     SIGNAL(toggled(bool)),   this, SLOT(setEffectsDirty()));
	connect( cbIconsOnButtons,     SIGNAL(toggled(bool)),   this, SLOT(setEffectsDirty()));
	connect( cbTearOffHandles,     SIGNAL(toggled(bool)),   this, SLOT(setEffectsDirty()));
	connect( comboToolbarIcons,    SIGNAL(activated(int)), this, SLOT(setToolbarsDirty()));

	addWhatsThis();

	// Insert the pages into the tabWidget
	tabWidget->addTab( page1, i18n("&Style"));
	tabWidget->addTab( page2, i18n("&Effects"));
	tabWidget->addTab( page3, i18n("&Toolbar"));

	//Enable/disable the button for the initial style
	updateConfigButton();
}


KCMStyle::~KCMStyle()
{
    qDeleteAll(styleEntries);
	delete appliedStyle;
}

void KCMStyle::updateConfigButton()
{
	if (!styleEntries[currentStyle()] || styleEntries[currentStyle()]->configPage.isEmpty()) {
		pbConfigStyle->setEnabled(false);
		return;
	}

	// We don't check whether it's loadable here -
	// lets us report an error and not waste time
	// loading things if the user doesn't click the button
	pbConfigStyle->setEnabled( true );
}

void KCMStyle::styleSpecificConfig()
{
	QString libname = styleEntries[currentStyle()]->configPage;

    KLibrary library(libname, KCMStyleFactory::componentData());
    if (!library.load()) {
		KMessageBox::detailedError(this,
			i18n("There was an error loading the configuration dialog for this style."),
            library.errorString(),
			i18n("Unable to Load Dialog"));
		return;
	}

    KLibrary::void_function_ptr allocPtr = library.resolveFunction("allocate_kstyle_config");

	if (!allocPtr)
	{
		KMessageBox::detailedError(this,
			i18n("There was an error loading the configuration dialog for this style."),
            library.errorString(),
			i18n("Unable to Load Dialog"));
		return;
	}

	//Create the container dialog
	StyleConfigDialog* dial = new StyleConfigDialog(this, styleEntries[currentStyle()]->name);
	dial->showButtonSeparator(true);

	typedef QWidget*(* factoryRoutine)( QWidget* parent );

	//Get the factory, and make the widget.
	factoryRoutine factory      = (factoryRoutine)(allocPtr); //Grmbl. So here I am on my
	//"never use C casts" moralizing streak, and I find that one can't go void* -> function ptr
	//even with a reinterpret_cast.

	QWidget*       pluginConfig = factory( dial );

	//Insert it in...
	dial->setMainWidget( pluginConfig );

	//..and connect it to the wrapper
	connect(pluginConfig, SIGNAL(changed(bool)), dial, SLOT(setDirty(bool)));
	connect(dial, SIGNAL(defaults()), pluginConfig, SLOT(defaults()));
	connect(dial, SIGNAL(save()), pluginConfig, SLOT(save()));

	if (dial->exec() == QDialog::Accepted  && dial->isDirty() ) {
		// Force re-rendering of the preview, to apply settings
		switchStyle(currentStyle(), true);

		//For now, ask all KDE apps to recreate their styles to apply the setitngs
		KGlobalSettings::self()->emitChange(KGlobalSettings::StyleChanged);

		// We call setStyleDirty here to make sure we force style re-creation
		setStyleDirty();
	}

	delete dial;
}

void KCMStyle::load()
{
	KConfig config( "kdeglobals", KConfig::FullConfig );
	// Page1 - Build up the Style ListBox
	loadStyle( config );

	// Page2 - Effects
	loadEffects( config );

	// Page3 - Misc.
	loadMisc( config );

	m_bEffectsDirty = false;
	m_bStyleDirty= false;
	m_bToolbarsDirty = false;

	emit changed( false );
}


void KCMStyle::save()
{
	// Don't do anything if we don't need to.
	if ( !(m_bToolbarsDirty | m_bEffectsDirty | m_bStyleDirty ) )
		return;

	bool allowMenuTransparency = false;
	bool allowMenuDropShadow   = false;

	// Read the KStyle flags to see if the style writer
	// has enabled menu translucency in the style.
	if (appliedStyle && appliedStyle->inherits("KStyle"))
	{
		allowMenuDropShadow = true;
/*		KStyle* style = dynamic_cast<KStyle*>(appliedStyle);
		if (style) {
			KStyle::KStyleFlags flags = style->styleFlags();
			if (flags & KStyle::AllowMenuTransparency)
				allowMenuTransparency = true;
		}*/
	}

	//### KDE4: not at all clear whether this will come back
	allowMenuTransparency = false;

	QString warn_string( i18n("<qt>Selected style: <b>%1</b><br /><br />"
		"One or more effects that you have chosen could not be applied because the selected "
		"style does not support them; they have therefore been disabled.<br />"
		"<br /></qt>", cbStyle->currentText()) );
	bool show_warning = false;

	// Warn the user if they're applying a style that doesn't support
	// menu translucency and they enabled it.
    if ( (!allowMenuTransparency) &&
		(cbEnableEffects->isChecked()) &&
		(comboMenuEffect->currentIndex() == 3) )	// Make Translucent
    {
		warn_string += i18n("Menu translucency is not available.<br />");
		comboMenuEffect->setCurrentIndex(0);    // Disable menu effect.
		show_warning = true;
	}

	if (!allowMenuDropShadow && cbMenuShadow->isChecked())
	{
		warn_string += i18n("Menu drop-shadows are not available.");
		cbMenuShadow->setChecked(false);
		show_warning = true;
	}

	// Tell the user what features we could not apply on their behalf.
	if (show_warning)
		KMessageBox::information(this, warn_string);


	// Save effects.
	KConfig _config( "kdeglobals" );
	KConfigGroup config(&_config, "KDE");

	config.writeEntry( "EffectsEnabled", cbEnableEffects->isChecked());
	int item = comboComboEffect->currentIndex();
	config.writeEntry( "EffectAnimateCombo", item == 1 );
	item = comboTooltipEffect->currentIndex();
	config.writeEntry( "EffectAnimateTooltip", item == 1);
	config.writeEntry( "EffectFadeTooltip", item == 2 );
	item = comboMenuHandle->currentIndex();
	config.writeEntry( "InsertTearOffHandle", item );
	item = comboMenuEffect->currentIndex();
	config.writeEntry( "EffectAnimateMenu", item == 1 );
	config.writeEntry( "EffectFadeMenu", item == 2 );

	// Handle KStyle's menu effects
	QString engine("Disabled");
	if (item == 3 && cbEnableEffects->isChecked())	// Make Translucent
		switch( comboMenuEffectType->currentIndex())
		{
			case 1: engine = "SoftwareBlend"; break;
			case 2: engine = "XRender"; break;
			default:
			case 0: engine = "SoftwareTint"; break;
		}

	{	// Braces force a QSettings::sync()
		QSettings settings;	// Only for KStyle stuff
		settings.setValue("/KStyle/Settings/MenuTransparencyEngine", engine);
		settings.setValue("/KStyle/Settings/MenuOpacity", slOpacity->value()/100.0);
 		settings.setValue("/KStyle/Settings/MenuDropShadow",
					cbEnableEffects->isChecked() && cbMenuShadow->isChecked() );
	}

	// Misc page
	config.writeEntry( "ShowIconsOnPushButtons", cbIconsOnButtons->isChecked(), KConfig::Normal|KConfig::Global);
	config.writeEntry( "EffectNoTooltip", !cbEnableTooltips->isChecked(), KConfig::Normal|KConfig::Global);

    KConfigGroup generalGroup(&_config, "General");
    generalGroup.writeEntry("widgetStyle", currentStyle());

    KConfigGroup toolbarStyleGroup(&_config, "Toolbar style");
    toolbarStyleGroup.writeEntry("Highlighting", cbHoverButtons->isChecked(), KConfig::Normal|KConfig::Global);
    toolbarStyleGroup.writeEntry("TransparentMoving", cbTransparentToolbars->isChecked(), KConfig::Normal|KConfig::Global);
	QString tbIcon;
	switch( comboToolbarIcons->currentIndex() )
	{
		case 0: tbIcon = "IconOnly"; break;
		case 1: tbIcon = "TextOnly"; break;
		case 2: tbIcon = "TextBesideIcon"; break;
		default: 
		case 3: tbIcon = "TextUnderIcon"; break;
	}
    toolbarStyleGroup.writeEntry("ToolButtonStyle", tbIcon, KConfig::Normal|KConfig::Global);
    _config.sync();

	// Export the changes we made to qtrc, and update all qt-only
	// applications on the fly, ensuring that we still follow the user's
	// export fonts/colors settings.
	if (m_bStyleDirty | m_bEffectsDirty)	// Export only if necessary
	{
		uint flags = KRdbExportQtSettings;
		KConfig _kconfig( "kcmdisplayrc", KConfig::CascadeConfig  );
		KConfigGroup kconfig(&_kconfig, "X11");
		bool exportKDEColors = kconfig.readEntry("exportKDEColors", true);
		if (exportKDEColors)
			flags |= KRdbExportColors;
		runRdb( flags );
	}

	// Now allow KDE apps to reconfigure themselves.
	if ( m_bStyleDirty )
		KGlobalSettings::self()->emitChange(KGlobalSettings::StyleChanged);

	if ( m_bToolbarsDirty )
		// ##### FIXME - Doesn't apply all settings correctly due to bugs in
		// KApplication/KToolbar
		KGlobalSettings::self()->emitChange(KGlobalSettings::ToolbarStyleChanged);

	if (m_bEffectsDirty) {
		KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged);
#ifdef Q_WS_X11
        // Send signal to all kwin instances
        QDBusMessage message =
           QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(message);
#endif
	}

	//update kicker to re-used tooltips kicker parameter otherwise, it overwritted
	//by style tooltips parameters.
	QByteArray data;
#ifdef __GNUC__
#warning "kde4: who is org.kde.kicker?"
#endif
#ifdef Q_WS_X11
	QDBusInterface kicker( "org.kde.kicker", "/kicker");
	kicker.call("configure");
#endif
	// Clean up
	m_bEffectsDirty  = false;
	m_bToolbarsDirty = false;
	m_bStyleDirty    = false;
	emit changed( false );
}


bool KCMStyle::findStyle( const QString& str, int& combobox_item )
{
	StyleEntry* se   = styleEntries[str.toLower()];

	QString     name = se ? se->name : str;

	combobox_item = 0;

	//look up name
	for( int i = 0; i < cbStyle->count(); i++ )
	{
		if ( cbStyle->itemText(i) == name )
		{
			combobox_item = i;
			return true;
		}
	}

	return false;
}


void KCMStyle::defaults()
{
	// Select default style
	int item = 0;
	bool found;

	found = findStyle( "plastique", item ); //### KStyle::defaultStyle()
	if (!found)
		found = findStyle( "highcolor", item );
	if (!found)
		found = findStyle( "default", item );
	if (!found)
		found = findStyle( "windows", item );
	if (!found)
		found = findStyle( "platinum", item );
	if (!found)
		found = findStyle( "motif", item );

	cbStyle->setCurrentIndex( item );

	m_bStyleDirty = true;
	switchStyle( currentStyle() );	// make resets visible

	// Effects..
	cbEnableEffects->setChecked(false);
	comboTooltipEffect->setCurrentIndex(0);
	comboComboEffect->setCurrentIndex(0);
	comboMenuEffect->setCurrentIndex(0);
	comboMenuHandle->setCurrentIndex(0);
	comboMenuEffectType->setCurrentIndex(0);
	slOpacity->setValue(90);
	cbMenuShadow->setChecked(false);

	// Miscellaneous
	cbHoverButtons->setChecked(true);
	cbTransparentToolbars->setChecked(true);
	cbEnableTooltips->setChecked(true);
	comboToolbarIcons->setCurrentIndex(0);
	cbIconsOnButtons->setChecked(true);
	cbTearOffHandles->setChecked(false);
}

void KCMStyle::setEffectsDirty()
{
	m_bEffectsDirty = true;
	emit changed(true);
}

void KCMStyle::setToolbarsDirty()
{
	m_bToolbarsDirty = true;
	emit changed(true);
}

void KCMStyle::setStyleDirty()
{
	m_bStyleDirty = true;
	emit changed(true);
}

// ----------------------------------------------------------------
// All the Style Switching / Preview stuff
// ----------------------------------------------------------------

void KCMStyle::loadStyle( KConfig& config )
{
	cbStyle->clear();

	// Create a dictionary of WidgetStyle to Name and Desc. mappings,
	// as well as the config page info
	qDeleteAll(styleEntries);
	styleEntries.clear();

	QString strWidgetStyle;
	QStringList list = KGlobal::dirs()->findAllResources("themes", "*.themerc",
														 KStandardDirs::Recursive |
														 KStandardDirs::NoDuplicates);
	for (QStringList::iterator it = list.begin(); it != list.end(); ++it)
	{
		KConfig config(  *it, KConfig::SimpleConfig);
		if ( !(config.hasGroup("KDE") && config.hasGroup("Misc")) )
			continue;

		KConfigGroup configGroup = config.group("KDE");

		strWidgetStyle = configGroup.readEntry("WidgetStyle");
		if (strWidgetStyle.isNull())
			continue;

		// We have a widgetstyle, so lets read the i18n entries for it...
		StyleEntry* entry = new StyleEntry;
		configGroup = config.group("Misc");
		entry->name = configGroup.readEntry("Name");
		entry->desc = configGroup.readEntry("Comment", i18n("No description available."));
		entry->configPage = configGroup.readEntry("ConfigPage", QString());

		// Check if this style should be shown
		configGroup = config.group("Desktop Entry");
		entry->hidden = configGroup.readEntry("Hidden", false);

		// Insert the entry into our dictionary.
		styleEntries.insert(strWidgetStyle.toLower(), entry);
	}

	// Obtain all style names
	QStringList allStyles = QStyleFactory::keys();

	// Get translated names, remove all hidden style entries.
	QStringList styles;
	StyleEntry* entry;
	for (QStringList::iterator it = allStyles.begin(); it != allStyles.end(); ++it)
	{
		QString id = (*it).toLower();
		// Find the entry.
		if ( (entry = styleEntries[id]) != 0 )
		{
			// Do not add hidden entries
			if (entry->hidden)
				continue;

			styles += entry->name;

			nameToStyleKey[entry->name] = id;
		}
		else
		{
			styles += (*it); //Fall back to the key (but in original case)
			nameToStyleKey[*it] = id;
		}
	}

	// Sort the style list, and add it to the combobox
	styles.sort();
	cbStyle->addItems( styles );

	// Find out which style is currently being used
	KConfigGroup configGroup = config.group( "General" );
	QString defaultStyle = "plastique"; //### KDE4: FIXME KStyle::defaultStyle();
	QString cfgStyle = configGroup.readEntry( "widgetStyle", defaultStyle );

	// Select the current style
	// Do not use cbStyle->listBox() as this may be NULL for some styles when
	// they use QPopupMenus for the drop-down list!

	// ##### Since Trolltech likes to seemingly copy & paste code,
	// QStringList::findItem() doesn't have a Qt::StringComparisonMode field.
	// We roll our own (yuck)
	cfgStyle = cfgStyle.toLower();
	int item = 0;
	for( int i = 0; i < cbStyle->count(); i++ )
	{
		QString id = nameToStyleKey[cbStyle->itemText(i)];
		item = i;
		if ( id == cfgStyle )	// ExactMatch
			break;
		else if ( id.contains( cfgStyle ) )
			break;
		else if ( id.contains( QApplication::style()->metaObject()->className() ) )
			break;
		item = 0;
	}
	cbStyle->setCurrentIndex( item );

	m_bStyleDirty = false;

	switchStyle( currentStyle() );	// make resets visible
}

QString KCMStyle::currentStyle()
{
	return nameToStyleKey[cbStyle->currentText()];
}


void KCMStyle::styleChanged()
{
	switchStyle( currentStyle() );
}


void KCMStyle::switchStyle(const QString& styleName, bool force)
{
	// Don't flicker the preview if the same style is chosen in the cb
	if (!force && appliedStyle && appliedStyle->objectName() == styleName)
		return;

	// Create an instance of the new style...
	QStyle* style = QStyleFactory::create(styleName);
	if (!style)
		return;

	// Prevent Qt from wrongly caching radio button images
	QPixmapCache::clear();

	setStyleRecursive( stylePreview, style );

	// this flickers, but reliably draws the widgets correctly.
	stylePreview->resize( stylePreview->sizeHint() );

	delete appliedStyle;
	appliedStyle = style;

	// Set the correct style description
	StyleEntry* entry = styleEntries[ styleName ];
	QString desc;
	desc = i18n("Description: %1", entry ? entry->desc : i18n("No description available.") );
	lblStyleDesc->setText( desc );
}

void KCMStyle::setStyleRecursive(QWidget* w, QStyle* s)
{
	// Don't let broken styles kill the palette
	// for other styles being previewed. (e.g SGI style)
	w->setPalette(QPalette());

	QPalette newPalette(KGlobalSettings::createApplicationPalette());
	s->polish( newPalette );
	w->setPalette(newPalette);

	// Apply the new style.
	w->setStyle(s);

	// Recursively update all children.
	const QObjectList children = w->children();

	// Apply the style to each child widget.
    foreach (QObject* child, children)
	{
		if (child->isWidgetType())
			setStyleRecursive((QWidget *) child, s);
	}
}


// ----------------------------------------------------------------
// All the Effects stuff
// ----------------------------------------------------------------

void KCMStyle::loadEffects( KConfig& config )
{
	// Load effects.
	KConfigGroup configGroup = config.group("KDE");

	cbEnableEffects->setChecked( configGroup.readEntry( "EffectsEnabled", false) );

	if ( configGroup.readEntry( "EffectAnimateCombo", false) )
		comboComboEffect->setCurrentIndex( 1 );
	else
		comboComboEffect->setCurrentIndex( 0 );

	if ( configGroup.readEntry( "EffectAnimateTooltip", false) )
		comboTooltipEffect->setCurrentIndex( 1 );
	else if ( configGroup.readEntry( "EffectFadeTooltip", false) )
		comboTooltipEffect->setCurrentIndex( 2 );
	else
		comboTooltipEffect->setCurrentIndex( 0 );

	if ( configGroup.readEntry( "EffectAnimateMenu", false) )
		comboMenuEffect->setCurrentIndex( 1 );
	else if ( configGroup.readEntry( "EffectFadeMenu", false) )
		comboMenuEffect->setCurrentIndex( 2 );
	else
		comboMenuEffect->setCurrentIndex( 0 );

	comboMenuHandle->setCurrentIndex(configGroup.readEntry("InsertTearOffHandle", 0));

	// KStyle Menu transparency and drop-shadow options...
	QSettings settings;
	QString effectEngine = settings.value("/KStyle/Settings/MenuTransparencyEngine", "Disabled" ).toString();

#ifdef HAVE_XRENDER
	if (effectEngine == "XRender") {
		comboMenuEffectType->setCurrentIndex(2);
		comboMenuEffect->setCurrentIndex(3);
	} else if (effectEngine == "SoftwareBlend") {
		comboMenuEffectType->setCurrentIndex(1);
		comboMenuEffect->setCurrentIndex(3);
#else
	if (effectEngine == "XRender" || effectEngine == "SoftwareBlend") {
		comboMenuEffectType->setCurrentIndex(1);	// Software Blend
		comboMenuEffect->setCurrentIndex(3);
#endif
	} else if (effectEngine == "SoftwareTint") {
		comboMenuEffectType->setCurrentIndex(0);
		comboMenuEffect->setCurrentIndex(3);
	} else
		comboMenuEffectType->setCurrentIndex(0);

	if (comboMenuEffect->currentIndex() != 3)	// If not translucency...
		menuPreview->setPreviewMode( MenuPreview::Tint );
	else if (comboMenuEffectType->currentIndex() == 0)
		menuPreview->setPreviewMode( MenuPreview::Tint );
	else
		menuPreview->setPreviewMode( MenuPreview::Blend );

	slOpacity->setValue( (int)(100 * settings.value("/KStyle/Settings/MenuOpacity", 0.90).toDouble()) );

	// Menu Drop-shadows...
	cbMenuShadow->setChecked( settings.value("/KStyle/Settings/MenuDropShadow", false).toBool() );

	if (cbEnableEffects->isChecked()) {
		containerFrame->setEnabled( true );
		menuContainer->setEnabled( comboMenuEffect->currentIndex() == 3 );
	} else {
		menuContainer->setEnabled( false );
		containerFrame->setEnabled( false );
	}

	m_bEffectsDirty = false;
}


void KCMStyle::menuEffectTypeChanged()
{
	MenuPreview::PreviewMode mode;

	if (comboMenuEffect->currentIndex() != 3)
		mode = MenuPreview::Tint;
	else if (comboMenuEffectType->currentIndex() == 0)
		mode = MenuPreview::Tint;
	else
		mode = MenuPreview::Blend;

	menuPreview->setPreviewMode(mode);

	m_bEffectsDirty = true;
}


void KCMStyle::menuEffectChanged()
{
	menuEffectChanged( cbEnableEffects->isChecked() );
	m_bEffectsDirty = true;
}


void KCMStyle::menuEffectChanged( bool enabled )
{
	if (enabled &&
		comboMenuEffect->currentIndex() == 3) {
		menuContainer->setEnabled(true);
	} else
		menuContainer->setEnabled(false);
	m_bEffectsDirty = true;
}


// ----------------------------------------------------------------
// All the Miscellaneous stuff
// ----------------------------------------------------------------

void KCMStyle::loadMisc( KConfig& config )
{
	// KDE's Part via KConfig
	KConfigGroup configGroup = config.group("Toolbar style");
	cbHoverButtons->setChecked(configGroup.readEntry("Highlighting", true));
	cbTransparentToolbars->setChecked(configGroup.readEntry("TransparentMoving", true));

	QString tbIcon = configGroup.readEntry("ToolButtonStyle", "TextUnderIcon");
	if (tbIcon == "TextOnly")
		comboToolbarIcons->setCurrentIndex(1);
	else if (tbIcon == "TextBesideIcon")
		comboToolbarIcons->setCurrentIndex(2);
	else if (tbIcon == "TextUnderIcon")
		comboToolbarIcons->setCurrentIndex(3);
	else
		comboToolbarIcons->setCurrentIndex(0);

	configGroup = config.group("KDE");
	cbIconsOnButtons->setChecked(configGroup.readEntry("ShowIconsOnPushButtons", false));
	cbEnableTooltips->setChecked(!configGroup.readEntry("EffectNoTooltip", false));
	cbTearOffHandles->setChecked(configGroup.readEntry("InsertTearOffHandle", false));

	m_bToolbarsDirty = false;
}

void KCMStyle::addWhatsThis()
{
	// Page1
	cbStyle->setWhatsThis( i18n("Here you can choose from a list of"
							" predefined widget styles (e.g. the way buttons are drawn) which"
							" may or may not be combined with a theme (additional information"
							" like a marble texture or a gradient).") );
	stylePreview->setWhatsThis( i18n("This area shows a preview of the currently selected style "
							"without having to apply it to the whole desktop.") );

	// Page2
	page2->setWhatsThis( i18n("This page allows you to enable various widget style effects. "
							"For best performance, it is advisable to disable all effects.") );
	cbEnableEffects->setWhatsThis( i18n( "If you check this box, you can select several effects "
							"for different widgets like combo boxes, menus or tooltips.") );
	comboComboEffect->setWhatsThis( i18n( "<p><b>Disable: </b>do not use any combo box effects.</p>\n"
							"<b>Animate: </b>Do some animation.") );
	comboTooltipEffect->setWhatsThis( i18n( "<p><b>Disable: </b>do not use any tooltip effects.</p>\n"
							"<p><b>Animate: </b>Do some animation.</p>\n"
							"<b>Fade: </b>Fade in tooltips using alpha-blending.") );
	comboMenuEffect->setWhatsThis( i18n( "<p><b>Disable: </b>do not use any menu effects.</p>\n"
							"<p><b>Animate: </b>Do some animation.</p>\n"
							"<p><b>Fade: </b>Fade in menus using alpha-blending.</p>\n"
							"<b>Make Translucent: </b>Alpha-blend menus for a see-through effect. (KDE styles only)") );
	cbMenuShadow->setWhatsThis( i18n( "When enabled, all popup menus will have a drop-shadow, otherwise "
							"drop-shadows will not be displayed. At present, only KDE styles can have this "
							"effect enabled.") );
	comboMenuEffectType->setWhatsThis( i18n( "<p><b>Software Tint: </b>Alpha-blend using a flat color.</p>\n"
							"<p><b>Software Blend: </b>Alpha-blend using an image.</p>\n"
							"<p><b>XRender Blend: </b>Use the XFree RENDER extension for image blending (if available). "
							"This method may be slower than the Software routines on non-accelerated displays, "
							"but may however improve performance on remote displays.</p>\n") );
	slOpacity->setWhatsThis( i18n("By adjusting this slider you can control the menu effect opacity.") );

	// Page3
	page3->setWhatsThis( i18n("<b>Note:</b> that all widgets in this combobox "
							"do not apply to Qt-only applications.") );
	cbHoverButtons->setWhatsThis( i18n("If this option is selected, toolbar buttons will change "
							"their color when the mouse cursor is moved over them." ) );
	cbTransparentToolbars->setWhatsThis( i18n("If you check this box, the toolbars will be "
							"transparent when moving them around.") );
	cbEnableTooltips->setWhatsThis( i18n( "If you check this option, the KDE application "
							"will offer tooltips when the cursor remains over items in the toolbar." ) );
	comboToolbarIcons->setWhatsThis( i18n( "<p><b>Icons only:</b> Shows only icons on toolbar buttons. "
							"Best option for low resolutions.</p>"
							"<p><b>Text only: </b>Shows only text on toolbar buttons.</p>"
							"<p><b>Text alongside icons: </b> Shows icons and text on toolbar buttons. "
							"Text is aligned alongside the icon.</p>"
							"<b>Text under icons: </b> Shows icons and text on toolbar buttons. "
							"Text is aligned below the icon.") );
	cbIconsOnButtons->setWhatsThis( i18n( "If you enable this option, KDE Applications will "
							"show small icons alongside some important buttons.") );
	cbTearOffHandles->setWhatsThis( i18n( "If you enable this option some pop-up menus will "
							"show so called tear-off handles. If you click them, you get the menu "
							"inside a widget. This can be very helpful when performing "
							"the same action multiple times.") );
}

#include "kcmstyle.moc"

// vim: set noet ts=4:
