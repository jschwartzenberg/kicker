#ifndef __MATRIX_H
#define __MATRIX_H

#include <X11/Xlib.h>

#include <qbuttongroup.h>
#include <qcolor.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qstring.h>

#include "kcolordlg.h"

#include "saver.h"

class KMatrixSaver;


class KMatrixSaverCfg {
 public:
  void readSettings();
  void writeSettings();

 public:
  // parameters
  QColor background, foreground;
  Bool mono;
  int density;
  int speed;
  QString insert;
};

class KMatrixSetup : public QDialog {
  Q_OBJECT
public:
  KMatrixSetup( QWidget *parent = NULL, const char *name = NULL );
  ~KMatrixSetup();

protected:
  inline void readSettings();
  void setSpeed(int val);
  void setDensity(int val);

private slots:
  void slotForegrColor();
  void slotBackgrColor();
  void slotDensity(int val);
  void slotDensityEdit(const char *s);
  void slotSpeed(int val);
  void slotSpeedEdit(const char *s);
  void slotInsert(int id);
  void slotOkPressed();
  void slotAbout();

private:
  QWidget *preview;
  KMatrixSaver *saver;
  QPushButton *backgrPush, *foregrPush;
  QLineEdit *densityEd, *speedEd;
  QSlider *speedSld, *densitySld;
  QButtonGroup *insertRbGrp;

  KMatrixSaverCfg cfg;
}; /* class KMatrixSetup : public QDialog */

extern const char progname[];

/* ok, I know, that it will be much better to implement this structures
   as classes but I don't want to do significant changes in original
   xmatrix code. Delta */
/* xmatrix data structures */
typedef struct m_cell_tag {
  int glyph;
  Bool changed;
  int glow;
} m_cell;

typedef struct m_feeder_tag {
  int remaining;
  int throttle;
  int y;
} m_feeder;

typedef struct m_state_tag {
  Display *dpy;
  Window window;
  XWindowAttributes xgwa;
  GC draw_gc, erase_gc;
  int grid_width, grid_height;
  int char_width, char_height;
  m_cell *cells;
  m_feeder *feeders;
  Bool insert_top_p, insert_bottom_p;
  int density;
  Pixmap images;
  int image_width, image_height;
  int nglyphs;
} m_state;

class KMatrixSaver : public kScreenSaver {
  Q_OBJECT
public:
  KMatrixSaver(Drawable drawable);
  virtual ~KMatrixSaver();

  void setBackgroundColor(const QColor &col);
  void setForegroundColor(const QColor &col);
  void setDensity(int val);
  void setSpeed(int val);
  void setInsert(QString val);

private:
  inline void readSettings();
  void blank();
  void shutdown_matrix();
  inline void restart_matrix();
  // original xmatrix functions
  void load_images();
  void init_matrix();
  void insert_glyph(int glyph, int x, int y);
  void feed_matrix();
  int densitizer();
  void hack_matrix();
  void draw_matrix();

protected slots:
  void slotTimeout();

private:
  m_state *state;
  QTimer timer;
  KMatrixSaverCfg cfg;
};


#endif

