// KDE Display fonts setup tab
//
// Copyright (c)  Mark Donohoe 1997
//                lars Knoll 1999
//                Rik Hemsley 2000
//
// Ported to kcontrol2 by Geert Jansen.

#include <config-workspace.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QtCore/QSettings>


//Added by qt3to4:
#include <QPixmap>
#include <QByteArray>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>


#include <kacceleratormanager.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kprocess.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <stdlib.h>

#include "../krdb/krdb.h"
#include "fonts.h"
#include "fonts.moc"

#include <kdebug.h>

#ifdef HAVE_FREETYPE
#include <ft2build.h>
#ifdef FT_LCD_FILTER_H
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#endif
#endif

#include <X11/Xlib.h>
#include <QX11Info>

#include <KPluginFactory>

// X11 headers
#undef Bool
#undef Unsorted
#undef None

static const char *aa_rgb_xpm[]={
"12 12 3 1",
"a c #0000ff",
"# c #00ff00",
". c #ff0000",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa"};
static const char *aa_bgr_xpm[]={
"12 12 3 1",
". c #0000ff",
"# c #00ff00",
"a c #ff0000",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa"};
static const char *aa_vrgb_xpm[]={
"12 12 3 1",
"a c #0000ff",
"# c #00ff00",
". c #ff0000",
"............",
"............",
"............",
"............",
"############",
"############",
"############",
"############",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa"};
static const char *aa_vbgr_xpm[]={
"12 12 3 1",
". c #0000ff",
"# c #00ff00",
"a c #ff0000",
"............",
"............",
"............",
"............",
"############",
"############",
"############",
"############",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa"};

static QPixmap aaPixmaps[]={ QPixmap(aa_rgb_xpm), QPixmap(aa_bgr_xpm), QPixmap(aa_vrgb_xpm), QPixmap(aa_vbgr_xpm) };

/**** DLL Interface ****/
K_PLUGIN_FACTORY(FontFactory, registerPlugin<KFonts>(); )
K_EXPORT_PLUGIN(FontFactory("kcmfonts"))

/**** FontUseItem ****/

FontUseItem::FontUseItem(
  QWidget * parent,
  const QString &name,
  const QString &grp,
  const QString &key,
  const QString &rc,
  const QFont &default_fnt,
  bool f
)
  : KFontRequester(parent, f),
    _rcfile(rc),
    _rcgroup(grp),
    _rckey(key),
    _default(default_fnt)
{
  KAcceleratorManager::setNoAccel( this );
  setTitle( name );
  readFont();
}

void FontUseItem::setDefault()
{
  setFont( _default, isFixedOnly() );
}

void FontUseItem::readFont()
{
  KConfig *config;

  bool deleteme = false;
  if (_rcfile.isEmpty())
    config = KGlobal::config().data();
  else
  {
    config = new KConfig(_rcfile);
    deleteme = true;
  }

  KConfigGroup group(config, _rcgroup);
  QFont tmpFnt(_default);
  setFont( group.readEntry(_rckey, tmpFnt), isFixedOnly() );
  if (deleteme) delete config;
}

void FontUseItem::writeFont()
{
  KConfig *config;

  if (_rcfile.isEmpty()) {
    config = KGlobal::config().data();
    KConfigGroup(config, _rcgroup).writeEntry(_rckey, font(), KConfig::Normal|KConfig::Global);
  } else {
    config = new KConfig(KStandardDirs::locateLocal("config", _rcfile));
    KConfigGroup(config, _rcgroup).writeEntry(_rckey, font());
    config->sync();
    delete config;
  }
}

void FontUseItem::applyFontDiff( const QFont &fnt, int fontDiffFlags )
{
  QFont _font( font() );

  if (fontDiffFlags & KFontChooser::FontDiffSize) {
    _font.setPointSize( fnt.pointSize() );
  }
  if (fontDiffFlags & KFontChooser::FontDiffFamily) {
    if (!isFixedOnly()) _font.setFamily( fnt.family() );
  }
  if (fontDiffFlags & KFontChooser::FontDiffStyle) {
    _font.setBold( fnt.bold() );
    _font.setItalic( fnt.italic() );
    _font.setUnderline( fnt.underline() );
  }

  setFont( _font, isFixedOnly() );
}

/**** FontAASettings ****/
#ifdef HAVE_FONTCONFIG
FontAASettings::FontAASettings(QWidget *parent)
              : KDialog( parent ),
                changesMade(false)
{
  setObjectName( "FontAASettings" );
  setModal( true );
  setCaption( i18n("Configure Anti-Alias Settings") );
  setButtons( Ok|Cancel );
  showButtonSeparator( true );

  QWidget     *mw=new QWidget(this);
  QGridLayout *layout=new QGridLayout(mw);
  layout->setSpacing(KDialog::spacingHint());
  layout->setMargin(0);

  excludeRange=new QCheckBox(i18n("E&xclude range:"), mw),
  layout->addWidget(excludeRange, 0, 0);
  excludeFrom=new KDoubleNumInput(0, 72, 8.0, mw,1, 1),
  excludeFrom->setSuffix(i18n(" pt"));
  layout->addWidget(excludeFrom, 0, 1);
  excludeToLabel=new QLabel(i18n(" to "), mw);
  layout->addWidget(excludeToLabel, 0, 2);
  excludeTo=new KDoubleNumInput(0, 72, 15.0, mw, 1, 1);
  excludeTo->setSuffix(i18n(" pt"));
  layout->addWidget(excludeTo, 0, 3);

  QString subPixelWhatsThis = i18n("<p>If you have a TFT or LCD screen you"
       " can further improve the quality of displayed fonts by selecting"
       " this option.<br />Sub-pixel rendering is also known as ClearType(tm).<br />"
       " In order for sub-pixel rendering to"
       " work correctly you need to know how the sub-pixels of your display"
       " are aligned.</p>"
       " <p>On TFT or LCD displays a single pixel is actually composed of"
       " three sub-pixels, red, green and blue. Most displays"
       " have a linear ordering of RGB sub-pixel, some have BGR.<br />"
       " This feature does not work with CRT monitors.</p>" );

  useSubPixel=new QCheckBox(i18n("&Use sub-pixel rendering:"), mw);
  layout->addWidget(useSubPixel, 1, 0);
  useSubPixel->setWhatsThis( subPixelWhatsThis );

  subPixelType=new QComboBox(mw);
  layout->addWidget(subPixelType, 1, 1, 1, 3);

  subPixelType->setEditable(false);
  subPixelType->setWhatsThis( subPixelWhatsThis );

  for(int t=KXftConfig::SubPixel::None+1; t<=KXftConfig::SubPixel::Vbgr; ++t)
    subPixelType->addItem(aaPixmaps[t-1], i18n(KXftConfig::description((KXftConfig::SubPixel::Type)t).toUtf8()));

  QLabel *hintingLabel=new QLabel(i18n("Hinting style: "), mw);
  layout->addWidget(hintingLabel, 2, 0);
  hintingStyle=new QComboBox(mw);
  hintingStyle->setEditable(false);
  layout->addWidget(hintingStyle, 2, 1, 1, 3);
  for(int s=KXftConfig::Hint::NotSet+1; s<=KXftConfig::Hint::Full; ++s)
    hintingStyle->addItem(i18n(KXftConfig::description((KXftConfig::Hint::Style)s).toUtf8()));

  QString hintingText(i18n("Hinting is a process used to enhance the quality of fonts at small sizes."));
  hintingStyle->setWhatsThis( hintingText);
  hintingLabel->setWhatsThis( hintingText);
  load();
  enableWidgets();
  setMainWidget(mw);

  connect(excludeRange, SIGNAL(toggled(bool)), SLOT(changed()));
  connect(useSubPixel, SIGNAL(toggled(bool)), SLOT(changed()));
  connect(excludeFrom, SIGNAL(valueChanged(double)), SLOT(changed()));
  connect(excludeTo, SIGNAL(valueChanged(double)), SLOT(changed()));
  connect(subPixelType, SIGNAL(activated(const QString &)), SLOT(changed()));
  connect(hintingStyle, SIGNAL(activated(const QString &)), SLOT(changed()));
}

bool FontAASettings::load()
{
  double     from, to;
  KXftConfig xft(KXftConfig::constStyleSettings);

  if(xft.getExcludeRange(from, to))
     excludeRange->setChecked(true);
  else
  {
    excludeRange->setChecked(false);
    from=8.0;
    to=15.0;
  }

  excludeFrom->setValue(from);
  excludeTo->setValue(to);

  KXftConfig::SubPixel::Type spType;

  if(!xft.getSubPixelType(spType) || KXftConfig::SubPixel::None==spType)
    useSubPixel->setChecked(false);
  else
  {
    int idx=getIndex(spType);

    if(idx>-1)
    {
      useSubPixel->setChecked(true);
      subPixelType->setCurrentIndex(idx);
    }
    else
      useSubPixel->setChecked(false);
  }

  KXftConfig::Hint::Style hStyle;

  if(!xft.getHintStyle(hStyle) || KXftConfig::Hint::NotSet==hStyle)
  {
    KConfig kglobals("kdeglobals", KConfig::CascadeConfig);

    hStyle=KXftConfig::Hint::Medium;
    xft.setHintStyle(hStyle);
    xft.apply();  // Save this setting
    KConfigGroup(&kglobals, "General").writeEntry("XftHintStyle", KXftConfig::toStr(hStyle));
    kglobals.sync();
    runRdb(KRdbExportXftSettings);
  }

  hintingStyle->setCurrentIndex(getIndex(hStyle));

  enableWidgets();

  return xft.getAntiAliasing();
}

bool FontAASettings::save( bool useAA )
{
  KXftConfig   xft(KXftConfig::constStyleSettings);
  KConfig      kglobals("kdeglobals", KConfig::CascadeConfig);
  KConfigGroup grp(&kglobals, "General");

  xft.setAntiAliasing( useAA );

  if(excludeRange->isChecked())
    xft.setExcludeRange(excludeFrom->value(), excludeTo->value());
  else
    xft.setExcludeRange(0, 0);

  KXftConfig::SubPixel::Type spType(useSubPixel->isChecked()
                                        ? getSubPixelType()
                                        : KXftConfig::SubPixel::None);

  xft.setSubPixelType(spType);
  grp.writeEntry("XftSubPixel", KXftConfig::toStr(spType));
  grp.writeEntry("XftAntialias", useAA);

  bool mod=false;
  KXftConfig::Hint::Style hStyle(getHintStyle());

  xft.setHintStyle(hStyle);

  QString hs(KXftConfig::toStr(hStyle));

  if(!hs.isEmpty() && hs!=grp.readEntry("XftHintStyle"))
  {
    grp.writeEntry("XftHintStyle", hs);
    mod=true;
  }
  kglobals.sync();

  if(!mod)
    mod=xft.changed();

  xft.apply();

  return mod;
}

void FontAASettings::defaults()
{
  excludeRange->setChecked(true);
  excludeFrom->setValue(8.0);
  excludeTo->setValue(15.0);
  useSubPixel->setChecked(false);
  hintingStyle->setCurrentIndex(getIndex(KXftConfig::Hint::Medium));
  enableWidgets();
}

int FontAASettings::getIndex(KXftConfig::SubPixel::Type spType)
{
  int pos=-1;
  int index;

  for(index=0; index<subPixelType->count(); ++index)
    if(subPixelType->itemText(index)==i18n(KXftConfig::description(spType).toUtf8()))
    {
      pos=index;
      break;
    }

  return pos;
}

KXftConfig::SubPixel::Type FontAASettings::getSubPixelType()
{
  int t;

  for(t=KXftConfig::SubPixel::None; t<=KXftConfig::SubPixel::Vbgr; ++t)
    if(subPixelType->currentText()==i18n(KXftConfig::description((KXftConfig::SubPixel::Type)t).toUtf8()))
      return (KXftConfig::SubPixel::Type)t;

  return KXftConfig::SubPixel::None;
}

int FontAASettings::getIndex(KXftConfig::Hint::Style hStyle)
{
  int pos=-1;
  int index;

  for(index=0; index<hintingStyle->count(); ++index)
    if(hintingStyle->itemText(index)==i18n(KXftConfig::description(hStyle).toUtf8()))
    {
      pos=index;
      break;
    }

  return pos;
}


KXftConfig::Hint::Style FontAASettings::getHintStyle()
{
  int s;

  for(s=KXftConfig::Hint::NotSet; s<=KXftConfig::Hint::Full; ++s)
    if(hintingStyle->currentText()==i18n(KXftConfig::description((KXftConfig::Hint::Style)s).toUtf8()))
      return (KXftConfig::Hint::Style)s;

  return KXftConfig::Hint::Medium;
}

void FontAASettings::enableWidgets()
{
  excludeFrom->setEnabled(excludeRange->isChecked());
  excludeTo->setEnabled(excludeRange->isChecked());
  excludeToLabel->setEnabled(excludeRange->isChecked());
  subPixelType->setEnabled(useSubPixel->isChecked());
#ifdef FT_LCD_FILTER_H
  static int ft_has_subpixel = -1;
  if( ft_has_subpixel == -1 ) {
    FT_Library            ftLibrary;
    if(FT_Init_FreeType(&ftLibrary) == 0) {
      ft_has_subpixel = ( FT_Library_SetLcdFilter(ftLibrary, FT_LCD_FILTER_DEFAULT )
        == FT_Err_Unimplemented_Feature ) ? 0 : 1;
      FT_Done_FreeType(ftLibrary);
    }
  }
  useSubPixel->setEnabled(ft_has_subpixel);
  subPixelType->setEnabled(ft_has_subpixel);
#endif
}
#endif

void FontAASettings::changed()
{
#ifdef HAVE_FONTCONFIG
    changesMade=true;
    enableWidgets();
#endif
}

#ifdef HAVE_FONTCONFIG
int FontAASettings::exec()
{
    int i=KDialog::exec();

    if(!i)
        load(); // Reset settings...

    return i && changesMade;
}
#endif

/**** KFonts ****/

static QString desktopConfigName()
{
  int desktop=0;
  if (QX11Info::display())
    desktop = DefaultScreen(QX11Info::display());
  QString name;
  if (desktop == 0)
    name = "kdesktoprc";
  else
    name.sprintf("kdesktop-screen-%drc", desktop);

  return name;
}

KFonts::KFonts(QWidget *parent, const QVariantList &args)
    :   KCModule(FontFactory::componentData(), parent, args)
{
  QStringList nameGroupKeyRc;

  nameGroupKeyRc
    << i18n("General")        << "General"    << "font"         << ""
    << i18n("Fixed width")    << "General"    << "fixed"        << ""
    << i18n("Toolbar")        << "General"    << "toolBarFont"  << ""
    << i18n("Menu")           << "General"    << "menuFont"     << ""
    << i18n("Window title")   << "WM"         << "activeFont"   << ""
    << i18n("Taskbar")        << "General"    << "taskbarFont"  << ""
    << i18n("Desktop")        << "FMSettings" << "StandardFont" << desktopConfigName();

  QList<QFont> defaultFontList;

  // Keep in sync with kdelibs/kdecore/kglobalsettings.cpp

  QFont f0("Sans Serif", 10);
  QFont f1("Monospace", 10);
  QFont f2("Sans Serif", 10);
  QFont f3("Sans Serif", 9, QFont::Bold);
  QFont f4("Sans Serif", 10);

  f0.setPointSize(10);
  f1.setPointSize(10);
  f2.setPointSize(10);
  f3.setPointSize(9);
  f4.setPointSize(10);

  defaultFontList << f0 << f1 << f2 << f0 << f3 << f4 << f0;

  QList<bool> fixedList;

  fixedList
    <<  false
    <<  true
    <<  false
    <<  false
    <<  false
    <<  false
    <<  false;

  QStringList quickHelpList;

  quickHelpList
    << i18n("Used for normal text (e.g. button labels, list items).")
    << i18n("A non-proportional font (i.e. typewriter font).")
    << i18n("Used to display text beside toolbar icons.")
    << i18n("Used by menu bars and popup menus.")
    << i18n("Used by the window titlebar.")
    << i18n("Used by the taskbar.")
    << i18n("Used for desktop icons.");

  QVBoxLayout * layout = new QVBoxLayout(this );
  layout->setSpacing( KDialog::spacingHint() );

  QGridLayout * fontUseLayout = new QGridLayout( );
  layout->addItem( fontUseLayout );
  fontUseLayout->setColumnStretch(0, 0);
  fontUseLayout->setColumnStretch(1, 1);
  fontUseLayout->setColumnStretch(2, 0);

  QList<QFont>::ConstIterator defaultFontIt(defaultFontList.begin());
  QList<bool>::ConstIterator fixedListIt(fixedList.begin());
  QStringList::ConstIterator quickHelpIt(quickHelpList.begin());
  QStringList::ConstIterator it(nameGroupKeyRc.begin());

  unsigned int count = 0;

  while (it != nameGroupKeyRc.end()) {

    QString name = *it; it++;
    QString group = *it; it++;
    QString key = *it; it++;
    QString file = *it; it++;

    FontUseItem * i =
      new FontUseItem(
        this,
        name,
        group,
        key,
        file,
        *defaultFontIt++,
        *fixedListIt++
      );

    fontUseList.append(i);
    connect(i, SIGNAL(fontSelected(const QFont &)), SLOT(fontSelected()));

    QLabel * fontUse = new QLabel(i18nc("Font role", "%1: ", name), this);
    fontUse->setWhatsThis( *quickHelpIt++);

    fontUseLayout->addWidget(fontUse, count, 0);
    fontUseLayout->addWidget(i, count, 1);

    ++count;
  }

   QHBoxLayout *hblay = new QHBoxLayout();
   layout->addItem(hblay);
   hblay->setSpacing(KDialog::spacingHint());
   hblay->addStretch();
   QPushButton * fontAdjustButton = new QPushButton(i18n("Ad&just All Fonts..."), this);
   fontAdjustButton->setWhatsThis( i18n("Click to change all fonts"));
   hblay->addWidget( fontAdjustButton );
   connect(fontAdjustButton, SIGNAL(clicked()), SLOT(slotApplyFontDiff()));

   layout->addSpacing(KDialog::spacingHint());

   QGridLayout* lay = new QGridLayout();
   layout->addItem(lay);
   lay->setColumnStretch( 3, 10 );
   lay->setSpacing(KDialog::spacingHint());
   QLabel* label=0L;
#ifdef HAVE_FONTCONFIG
   label = new QLabel( i18n( "Use a&nti-aliasing:" ), this );
   lay->addWidget( label, 0, 0 );
   cbAA = new QComboBox( this );
   cbAA->insertItem( AAEnabled, i18nc( "Use anti-aliasing", "Enabled" )); // change AASetting type if order changes
   cbAA->insertItem( AASystem, i18nc( "Use anti-aliasing", "System settings" ));
   cbAA->insertItem( AADisabled, i18nc( "Use annti-aliasing", "Disabled" ));
   cbAA->setWhatsThis( i18n("If this option is selected, KDE will smooth the edges of curves in "
                              "fonts."));
   aaSettingsButton = new QPushButton( i18n( "Configure..." ), this);
   connect(aaSettingsButton, SIGNAL(clicked()), SLOT(slotCfgAa()));
   label->setBuddy( cbAA );
   lay->addWidget( cbAA, 0, 1 );
   lay->addWidget( aaSettingsButton, 0, 2 );
   connect(cbAA, SIGNAL(activated(int)), SLOT(slotUseAntiAliasing()));
#endif
   label = new QLabel( i18n( "Force fonts DPI:" ), this );
   lay->addWidget( label, 1, 0 );
   comboForceDpi = new QComboBox( this );
   label->setBuddy( comboForceDpi );
   comboForceDpi->insertItem( DPINone, i18nc("Force fonts DPI", "Disabled" )); // change DPISetti ng type if order changes
   comboForceDpi->insertItem( DPI96, i18n( "96 DPI" ));
   comboForceDpi->insertItem( DPI120, i18n( "120 DPI" ));
   QString whatsthis = i18n(
       "<p>This option forces a specific DPI value for fonts. It may be useful"
       " when the real DPI of the hardware is not detected properly and it"
       " is also often misused when poor quality fonts are used that do not"
       " look well with DPI values other than 96 or 120 DPI.</p>"
       "<p>The use of this option is generally discouraged. For selecting proper DPI"
       " value a better option is explicitly configuring it for the whole X server if"
       " possible (e.g. DisplaySize in xorg.conf or adding <i>-dpi value</i> to"
       " ServerLocalArgs= in $KDEDIR/share/config/kdm/kdmrc). When fonts do not render"
       " properly with real DPI value better fonts should be used or configuration"
       " of font hinting should be checked.</p>" );
   comboForceDpi->setWhatsThis(whatsthis);
   connect( comboForceDpi, SIGNAL( activated( int )), SLOT( changed()));
   lay->addWidget( comboForceDpi, 1, 1 );

   layout->addStretch(1);

#ifdef HAVE_FONTCONFIG
   aaSettings=new FontAASettings(this);
#endif

   load();
}

KFonts::~KFonts()
{
  QList<FontUseItem *>::Iterator it(fontUseList.begin()),
                                 end(fontUseList.end());

  for(; it!=end; ++it)
    delete (*it);
  fontUseList.clear();
}

void KFonts::fontSelected()
{
  emit changed(true);
}

void KFonts::defaults()
{
  for ( int i = 0; i < (int) fontUseList.count(); i++ )
    fontUseList.at( i )->setDefault();

#ifdef HAVE_FONTCONFIG
  useAA = AASystem;
  cbAA->setCurrentIndex( useAA );
  aaSettings->defaults();
#endif
  comboForceDpi->setCurrentIndex( DPINone );
  emit changed(true);
}

void KFonts::load()
{
  QList<FontUseItem *>::Iterator it(fontUseList.begin()),
                                 end(fontUseList.end());

  for(; it!=end; ++it)
    (*it)->readFont();

#ifdef HAVE_FONTCONFIG
  useAA_original = useAA = aaSettings->load() ? AAEnabled : AADisabled;
  cbAA->setCurrentIndex( useAA );
#endif

  KConfig _cfgfonts( "kcmfonts" );
  KConfigGroup cfgfonts(&_cfgfonts, "General");
  int dpicfg = cfgfonts.readEntry( "forceFontDPI", 0 );
  DPISetting dpi = dpicfg == 120 ? DPI120 : dpicfg == 96 ? DPI96 : DPINone;
  comboForceDpi->setCurrentIndex( dpi );
  dpi_original = dpi;
#ifdef HAVE_FONTCONFIG
  if( cfgfonts.readEntry( "dontChangeAASettings", true )) {
      useAA_original = useAA = AASystem;
      cbAA->setCurrentIndex( useAA );
  }
  aaSettingsButton->setEnabled( cbAA->currentIndex() == AAEnabled );
#endif

  emit changed(false);
}

void KFonts::save()
{
  QList<FontUseItem *>::Iterator it(fontUseList.begin()),
                                 end(fontUseList.end());

  for(; it!=end; ++it)
      (*it)->writeFont();

  KGlobal::config()->sync();

  KConfig _cfgfonts( "kcmfonts" );
  KConfigGroup cfgfonts(&_cfgfonts, "General");
  DPISetting dpi = static_cast< DPISetting >( comboForceDpi->currentIndex());
  const int dpi2value[] = { 0, 96, 120 };
  cfgfonts.writeEntry( "forceFontDPI", dpi2value[ dpi ] );
#ifdef HAVE_FONTCONFIG
  cfgfonts.writeEntry( "dontChangeAASettings", cbAA->currentIndex() == AASystem );
#endif
  cfgfonts.sync();
  // if the setting is reset in the module, remove the dpi value,
  // otherwise don't explicitly remove it and leave any possible system-wide value
  if( dpi == DPINone && dpi_original != DPINone ) {
      KProcess proc;
      proc << "xrdb" << "-quiet" << "-remove" << "-nocpp";
      proc.start();
      if (proc.waitForStarted()) {
        proc.write( QByteArray( "Xft.dpi\n" ) );
        proc.closeWriteChannel();
        proc.waitForFinished();
      }
  }

  // KDE-1.x support
  {
  KConfig config( QDir::homePath() + "/.kderc", KConfig::SimpleConfig);
  KConfigGroup grp(&config, "General");

  for(it=fontUseList.begin(); it!=end; ++it) {
      if("font"==(*it)->rcKey())
          QSettings().setValue("/qt/font", (*it)->font().toString());
      kDebug(1208) << "write entry " <<  (*it)->rcKey();
      grp.writeEntry( (*it)->rcKey(), (*it)->font() );
  }
  config.sync();
  }

  KGlobalSettings::self()->emitChange(KGlobalSettings::FontChanged);

  kapp->processEvents(); // Process font change ourselves


  // Don't overwrite global settings unless explicitly asked for - e.g. the system
  // fontconfig setup may be much more complex than this module can provide.
  // TODO: With AASystem the changes already made by this module should be reverted somehow.
#ifdef HAVE_FONTCONFIG
  bool aaSave = false;
  if( cbAA->currentIndex() != AASystem )
      aaSave = aaSettings->save( useAA == AAEnabled );

  if( aaSave || (useAA != useAA_original) || dpi != dpi_original) {
    KMessageBox::information(this,
      i18n(
        "<p>Some changes such as anti-aliasing will only affect newly started applications.</p>"
      ), i18n("Font Settings Changed"), "FontSettingsChanged", false);
    useAA_original = useAA;
    dpi_original = dpi;
  }
#else
  if(dpi != dpi_original) {
    KMessageBox::information(this,
      i18n(
        "<p>Some changes such as DPI will only affect newly started applications.</p>"
      ), i18n("Font Settings Changed"), "FontSettingsChanged", false);
    dpi_original = dpi;
  }
#endif
  runRdb(KRdbExportXftSettings);

  emit changed(false);
}


void KFonts::slotApplyFontDiff()
{
  QFont font = QFont(fontUseList.first()->font());
	KFontChooser::FontDiffFlags fontDiffFlags = 0;
  int ret = KFontDialog::getFontDiff(font,fontDiffFlags);

  if (ret == KDialog::Accepted && fontDiffFlags)
  {
    for ( int i = 0; i < (int) fontUseList.count(); i++ )
      fontUseList.at( i )->applyFontDiff( font,fontDiffFlags );
    emit changed(true);
  }
}

void KFonts::slotUseAntiAliasing()
{
#ifdef HAVE_FONTCONFIG
    useAA = static_cast< AASetting >( cbAA->currentIndex());
    aaSettingsButton->setEnabled( cbAA->currentIndex() == AAEnabled );
    emit changed(true);
#endif
}

void KFonts::slotCfgAa()
{
#ifdef HAVE_FONTCONFIG
  if(aaSettings->exec())
  {
    emit changed(true);
  }
#endif
}

// vim:ts=2:sw=2:tw=78
