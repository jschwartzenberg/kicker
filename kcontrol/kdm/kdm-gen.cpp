/*
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdm-gen.h"

#include "kbackedcombobox.h"

#include <KDialog>
#include <KFontRequester>
#include <KLocale>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KLanguageButton>

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>

extern KConfig *config;

KDMGeneralWidget::KDMGeneralWidget( QWidget *parent )
	: QWidget( parent )
{
	QString wtstr;

	QBoxLayout *ml = new QVBoxLayout( this );
	ml->setSpacing( KDialog::spacingHint() );
	ml->setMargin( KDialog::marginHint() );

	QGroupBox *box = new QGroupBox( "Appearance", this );
	ml->addWidget( box );
	QGridLayout *grid = new QGridLayout( box );
	grid->setSpacing( KDialog::spacingHint() );
	grid->setMargin( KDialog::marginHint() );
	grid->setColStretch( 2, 1 );

	useThemeCheck = new QCheckBox( i18n("&Use themed greeter"), box );
	connect( useThemeCheck, SIGNAL(toggled( bool )), SLOT(slotUseThemeChanged()) );
	useThemeCheck->setWhatsThis( i18n("Enable this if you would like to use a themed Login Manager.") );
	grid->addWidget( useThemeCheck, 0, 0, 1, 2 );

	guicombo = new KBackedComboBox( box );
	guicombo->insertItem( "", i18n("<placeholder>default</placeholder>") );
	loadGuiStyles( guicombo );
	QLabel *label = new QLabel( i18n("GUI s&tyle:"), box );
	label->setBuddy( guicombo );
	connect( guicombo, SIGNAL(activated( int )), SIGNAL(changed()) );
	grid->addWidget( label, 1, 0 );
	grid->addWidget( guicombo, 1, 1 );
	wtstr = i18n("You can choose a basic GUI style here that will be "
	             "used by KDM only.");
	label->setWhatsThis( wtstr );
	guicombo->setWhatsThis( wtstr );

	colcombo = new KBackedComboBox( box );
	colcombo->insertItem( "", i18n("<placeholder>default</placeholder>") );
	loadColorSchemes( colcombo );
	label = new QLabel( i18n("Color sche&me:"), box );
	label->setBuddy( colcombo );
	connect( colcombo, SIGNAL(activated( int )), SIGNAL(changed()) );
	grid->addWidget( label, 2, 0 );
	grid->addWidget( colcombo, 2, 1 );
	wtstr = i18n("You can choose a basic Color Scheme here that will be "
	             "used by KDM only.");
	label->setWhatsThis( wtstr );
	colcombo->setWhatsThis( wtstr );

	box = new QGroupBox( "Locale", this );
	ml->addWidget( box );
	grid = new QGridLayout( box );
	grid->setSpacing( KDialog::spacingHint() );
	grid->setMargin( KDialog::marginHint() );
	grid->setColStretch( 2, 1 );

	// The Language group box
	langcombo = new KLanguageButton( box );
	langcombo->showLanguageCodes(true);
	langcombo->loadAllLanguages();
	connect( langcombo, SIGNAL(activated( const QString & )), SIGNAL(changed()) );
	label = new QLabel( i18n("&Language:"), this );
	label->setBuddy( langcombo );
	grid->addWidget( label, 0, 0 );
	grid->addWidget( langcombo, 0, 1 );
	wtstr = i18n("Here you can choose the language used by KDM. This setting does not affect"
	             " a user's personal settings; that will take effect after login.");
	label->setWhatsThis( wtstr );
	langcombo->setWhatsThis( wtstr );

	box = new QGroupBox( "Fonts", this );
	ml->addWidget( box );
	grid = new QGridLayout( box );
	grid->setSpacing( KDialog::spacingHint() );
	grid->setMargin( KDialog::marginHint() );

	label = new QLabel( i18n("&General:"), box );
	stdFontChooser = new KFontRequester( box );
	label->setBuddy( stdFontChooser );
	stdFontChooser->setWhatsThis( i18n("This changes the font which is used for all the text in the login manager except for the greeting and failure messages.") );
	connect( stdFontChooser, SIGNAL(fontSelected( const QFont& )), SIGNAL(changed()) );
	grid->addWidget( label, 0, 0 );
	grid->addWidget( stdFontChooser, 0, 1 );

	label = new QLabel( i18n("&Failure font:"), box );
	failFontChooser = new KFontRequester( box );
	label->setBuddy( failFontChooser );
	failFontChooser->setWhatsThis( i18n("This changes the font which is used for failure messages in the login manager.") );
	connect( failFontChooser, SIGNAL(fontSelected( const QFont& )), SIGNAL(changed()) );
	grid->addWidget( label, 1, 0 );
	grid->addWidget( failFontChooser, 1, 1 );

	label = new QLabel( i18n("Gree&ting:"), box );
	greetingFontChooser = new KFontRequester( box );
	label->setBuddy( greetingFontChooser );
	greetingFontChooser->setWhatsThis( i18n("This changes the font which is used for the login manager's greeting.") );
	connect( greetingFontChooser, SIGNAL(fontSelected( const QFont& )), SIGNAL(changed()) );
	grid->addWidget( label, 2, 0 );
	grid->addWidget( greetingFontChooser, 2, 1 );

	aacb = new QCheckBox( i18n("Use anti-aliasing for fonts"), box );
	aacb->setWhatsThis( i18n("If you check this box and your X-Server has the Xft extension, "
	                         "fonts will be antialiased (smoothed) in the login dialog.") );
	connect( aacb, SIGNAL(toggled( bool )), SIGNAL(changed()) );
	grid->addWidget( aacb, 3, 0, 1, 2 );

	ml->addStretch( 1 );
}

void KDMGeneralWidget::makeReadOnly()
{
	useThemeCheck->setEnabled( false );
	guicombo->setEnabled( false );
	colcombo->setEnabled( false );
	langcombo->setEnabled( false );
	stdFontChooser->button()->setEnabled( false );
	failFontChooser->button()->setEnabled( false );
	greetingFontChooser->button()->setEnabled( false );
	aacb->setEnabled( false );
}

void KDMGeneralWidget::loadColorSchemes( KBackedComboBox *combo )
{
	// XXX: Global + local schemes
	QStringList list = KGlobal::dirs()->
		findAllResources( "data", "kdisplay/color-schemes/*.kcsrc", KStandardDirs::NoDuplicates );
	for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
	{
		KConfig _config( *it, KConfig::SimpleConfig );
		KConfigGroup config(&_config, "Color Scheme" );

		QString str;
		if (!(str = config.readEntry( "Name" )).isEmpty() ||
			!(str = config.readEntry( "name" )).isEmpty())
		{
			QString str2 = (*it).mid( (*it).lastIndexOf( '/' ) + 1 ); // strip off path
			str2.resize( str2.length() - 6 ); // strip off ".kcsrc
				combo->insertItem( str2, str );
		}
	}
}

void KDMGeneralWidget::loadGuiStyles(KBackedComboBox *combo)
{
	// XXX: Global + local schemes
	QStringList list = KGlobal::dirs()->
		findAllResources( "data", "kstyle/themes/*.themerc", KStandardDirs::NoDuplicates );
	for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
	{
		KConfig config( *it, KConfig::SimpleConfig);

		if (!(config.hasGroup( "KDE" ) && config.hasGroup( "Misc" )))
			continue;

		if (config.group( "Desktop Entry" ).readEntry( "Hidden" , false ))
			continue;

		QString str2 = config.group( "KDE" ).readEntry( "WidgetStyle" );
		if (str2.isNull())
			continue;

		combo->insertItem( str2, config.group( "Misc" ).readEntry( "Name" ) );
	}
}

void KDMGeneralWidget::set_def()
{
	stdFontChooser->setFont( QFont( "Sans Serif", 10 ) );
	failFontChooser->setFont( QFont( "Sans Serif", 10, QFont::Bold ) );
	greetingFontChooser->setFont( QFont( "Serif", 20 ) );
}

void KDMGeneralWidget::save()
{
	KConfigGroup configGrp = config->group( "X-*-Greeter" );

	configGrp.writeEntry( "UseTheme", useThemeCheck->isChecked() );
	configGrp.writeEntry( "GUIStyle", guicombo->currentId() );
	configGrp.writeEntry( "ColorScheme", colcombo->currentId() );
	configGrp.writeEntry( "Language", langcombo->current() );
	configGrp.writeEntry( "StdFont", stdFontChooser->font() );
	configGrp.writeEntry( "GreetFont", greetingFontChooser->font() );
	configGrp.writeEntry( "FailFont", failFontChooser->font() );
	configGrp.writeEntry( "AntiAliasing", aacb->isChecked() );
}


void KDMGeneralWidget::load()
{
	set_def();

	KConfigGroup configGrp = config->group( "X-*-Greeter" );

	useThemeCheck->setChecked( configGrp.readEntry( "UseTheme", false ) );

	// Check the GUI type
	guicombo->setCurrentId( configGrp.readEntry( "GUIStyle" ) );

	// Check the Color Scheme
	colcombo->setCurrentId( configGrp.readEntry("ColorScheme" ) );

	// get the language
	langcombo->setCurrentItem( configGrp.readEntry( "Language", "C" ) );

	// Read the fonts
	QFont font = stdFontChooser->font();
	stdFontChooser->setFont( configGrp.readEntry( "StdFont", font ) );
	font = failFontChooser->font();
	failFontChooser->setFont( configGrp.readEntry( "FailFont", font ) );
	font = greetingFontChooser->font();
	greetingFontChooser->setFont( configGrp.readEntry( "GreetFont", font ) );

	aacb->setChecked( configGrp.readEntry( "AntiAliasing", false ) );
}


void KDMGeneralWidget::defaults()
{
	useThemeCheck->setChecked( false );
	guicombo->setCurrentId( "" );
	colcombo->setCurrentId( "" );
	langcombo->setCurrentItem( "en_US" );
	set_def();
	aacb->setChecked( false );
}

void KDMGeneralWidget::slotUseThemeChanged()
{
	bool en = useThemeCheck->isChecked();
	failFontChooser->setEnabled( !en );
	greetingFontChooser->setEnabled( !en );
	emit useThemeChanged( en );
	emit changed();
}

#include "kdm-gen.moc"
