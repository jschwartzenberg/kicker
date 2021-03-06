/*****************************************************************

Copyright (c) 2004 Zack Rusin <zrusin@kde.org>
                   Sami Kyostil <skyostil@kempele.fi>
                   Aaron J. Seigo <aseigo@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <QApplication>
#include <QBitmap>
#include <QPainter>
#include <QTimer>

#include <QTextLayout>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

#include <kdialog.h>

#include "utils.h"

#include "kickertip.h"
#include "kickerSettings.h"

class KickerTip::Private
{
public:
    Private()
     : document(0),
       dissolveSize(0),
       dissolveDelta(-1),
       direction(Plasma::Up),
       dirty(false),
       tippingFor(0)
    {}

    QBitmap mask;
    QPixmap pixmap;
    QPixmap icon;
    MaskEffect maskEffect;
    QTextDocument document;

    int dissolveSize;
    int dissolveDelta;
    Plasma::Position direction;

    QTimer timer;
    QTimer frameTimer;
    bool dirty;

    const QWidget* tippingFor;
};

// putting this #include higher results in compile errors
#include <netwm.h>

static const int DEFAULT_FRAMES_PER_SECOND = 30;

KickerTip* KickerTip::m_self = 0;
int KickerTip::m_tippingEnabled = 1;

void KickerTip::Client::updateTip() const
{
    if (KickerTip::self()->isTippingFor(dynamic_cast<const QWidget*>(this)) &&
        KickerTip::self()->isVisible())
    {
        KickerTip::self()->display();
    }
}

KickerTip* KickerTip::self()
{
    if (!m_self)
    {
        m_self = new KickerTip(0);
    }

    return m_self;
}

KickerTip::KickerTip(QWidget * parent)
    : QWidget(parent, Qt::X11BypassWindowManagerHint)
{
    d = new Private;
    d->timer.setSingleShot(true);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setPaletteBackgroundColor(palette().color( QPalette::Background ) );
    resize(0, 0);
    hide();
    connect(&d->frameTimer, SIGNAL(timeout()), SLOT(internalUpdate()));
    setAttribute(Qt::WA_OpaquePaintEvent);
}

KickerTip::~KickerTip()
{
    delete d;
}

void KickerTip::display()
{
    if (!tippingEnabled())
    {
        // prevent tips from showing when the active window is fullscreened
        NETRootInfo ri(QX11Info::display(), NET::ActiveWindow);
        NETWinInfo wi(QX11Info::display(), ri.activeWindow(), ri.rootWindow(), NET::WMState);
        if (wi.state() & NET::FullScreen)
        {
            return;
        }
    }

    QWidget *widget = const_cast<QWidget*>(d->tippingFor);
    KickerTip::Client *client = dynamic_cast<KickerTip::Client*>(widget);

    if (!client)
    {
        return;
    }

    // Declare interchange object and define defaults.
    Data data;
    data.maskEffect = Dissolve;
    data.duration = 4000;
    data.direction = Plasma::Up;

    // Tickle the information out of the bastard.
    client->updateTipData(data);

    if (data.message.isEmpty() && data.subtext.isEmpty() && data.icon.isNull())
    {
        return;
    }

    d->document.setHtml("<h3>" + data.message + "</h3><p>" +
                        data.subtext + "</p>");
    d->document.setTextWidth(400);
    d->direction = data.direction;

    if (KickerSettings::mouseOversShowIcon())
    {
        d->icon = data.icon;
    }
    else if (KickerSettings::mouseOversShowText())
    {
        d->icon = QPixmap();
    }
    else
    {
        // don't bother since we have NOTHING to show
        return;
    }

    d->maskEffect = isVisible() ? Plain : data.maskEffect;
    d->dissolveSize = 24;
    d->dissolveDelta = -1;

    displayInternal();

    d->frameTimer.start(1000 / DEFAULT_FRAMES_PER_SECOND);

    // close the message window after given mS
    if (data.duration > 0)
    {
        disconnect(&d->timer, SIGNAL(timeout()), 0, 0);
        connect(&d->timer, SIGNAL(timeout()), SLOT(hide()));
        d->timer.start(data.duration);
    }
    else
    {
        d->timer.stop();
    }

    move(Plasma::popupPosition(d->direction, this, d->tippingFor));
    show();
}

void KickerTip::paintEvent(QPaintEvent * e)
{
    if (d->dirty)
    {
        displayInternal();
        d->dirty = false;
    }

    QPainter p(this);
    p.drawPixmap(e->rect().topLeft(), d->pixmap, e->rect());
}

void KickerTip::mousePressEvent(QMouseEvent * /*e*/)
{
    d->timer.stop();
    d->frameTimer.stop();
    hide();
}

void KickerTip::drawBackground(QPainter *p, const QColor &outline, const QBrush &fill) const
{
    const QRect r = d->pixmap.rect().adjusted(0, 0, -1, -1);

    p->setBrush(fill);
    p->setPen(QPen(outline, 1));
    p->drawRoundRect(r, 1600 / r.width(), 1600 / r.height());
}

void KickerTip::plainMask()
{
    QPainter maskPainter(&d->mask);

    d->mask.fill(Qt::color0);
    drawBackground(&maskPainter, Qt::color1, Qt::color1);    

    setMask(d->mask);
    d->frameTimer.stop();
}

void KickerTip::dissolveMask()
{
    QPainter maskPainter(&d->mask);

    d->mask.fill(Qt::color0);
    drawBackground(&maskPainter, Qt::color1, Qt::color1);

    d->dissolveSize += d->dissolveDelta;

    if (d->dissolveSize > 0)
    {
        maskPainter.setBrush(Qt::color0);
        maskPainter.setPen(Qt::NoPen);

        int x, y, s;
        const int size = 16;

        for (y = 0; y < height() + size; y += size)
        {
            x = width();
            s = d->dissolveSize * x / 128;
            for (; x > -size; x -= size, s -= 2)
            {
                if (s < 0)
                {
                    break;
                }
                maskPainter.drawEllipse(x - s / 2, y - s / 2, s, s);
            }
        }
    }
    else if (d->dissolveSize < 0)
    {
        d->frameTimer.stop();
        d->dissolveDelta = 1;
    }

    setMask(d->mask);
}

void KickerTip::displayInternal()
{
    // we need to check for d->tippingFor here as well as d->document
    // since if one is really persistant and moves the mouse around very fast
    // you can trigger a situation where d->tippingFor gets reset to 0 but
    // before display() is called!
    if (!d->tippingFor || d->document.isEmpty())
    {
        return;
    }

    // determine text rectangle
    QRect textRect(0, 0, 0, 0);
    if (KickerSettings::mouseOversShowText())
    {
        textRect.setWidth(qRound(d->document.idealWidth()));
        textRect.setHeight(qRound(d->document.size().height()));
    }

    textRect.translate(-textRect.left(), -textRect.top());
    textRect.adjust(0, 0, 2, 2);

    int margin = KDialog::marginHint();
    int height = qMax(d->icon.height(), textRect.height()) + 2 * margin;
    int textX = d->icon.isNull() ? margin : 2 + d->icon.width() + 2 * margin;
    int width = textX + textRect.width() + margin;
    int textY = (height - textRect.height()) / 2;

    // resize pixmap, mask and widget
    bool firstTime = d->dissolveSize == 24;
    if (firstTime)
    {
        d->mask = QBitmap(width, height);
        d->pixmap = QPixmap(width, height);
        resize(width, height);
        if (isVisible())
        {
            // we've already been shown before, but we may grow larger.
            // in the case of Up or Right displaying tips, this growth can
            // result in the tip occluding the panel and causing it to redraw
            // once we return back to display() causing horrid flicker
            move(Plasma::popupPosition(d->direction, this, d->tippingFor));
        }
    }

    // create and set transparency mask
    switch (d->maskEffect)
    {
        case Plain:
            plainMask();
        break;

        case Dissolve:
            dissolveMask();
        break;
    }

    // draw background
    QPainter bufferPainter(&d->pixmap);
    drawBackground(&bufferPainter, Qt::black, backgroundColor());

    // draw icon if present
    if (!d->icon.isNull())
    {
        bufferPainter.drawPixmap(margin,
                                 margin,
                                 d->icon, 0, 0,
                                 d->icon.width(), d->icon.height());
    }

    if (KickerSettings::mouseOversShowText())
    {
        int shadowOffset = QApplication::isRightToLeft() ? -1 : 1;
        QPointF posShadow = QPointF(5 + textX + shadowOffset, textY + 1);
        QPointF posText = QPointF(5 + textX, textY);

        QAbstractTextDocumentLayout::PaintContext ctx;
        ctx.palette = palette();
        ctx.palette.setColor(QPalette::Text, ctx.palette.color(QPalette::Background).dark(115));
        bufferPainter.save();
        bufferPainter.translate(posShadow);
        d->document.documentLayout()->draw(&bufferPainter, ctx);
        bufferPainter.restore();

        ctx.palette = palette();
        bufferPainter.save();
        bufferPainter.translate(posText);
        d->document.documentLayout()->draw(&bufferPainter, ctx);
        bufferPainter.restore();
    }
}

void KickerTip::tipFor(const QWidget* w)
{
    if (d->tippingFor)
    {
        disconnect(d->tippingFor, SIGNAL(destroyed(QObject*)),
                   this, SLOT(tipperDestroyed(QObject*)));
    }

    d->tippingFor = w;

    if (d->tippingFor)
    {
        connect(d->tippingFor, SIGNAL(destroyed(QObject*)),
                this, SLOT(tipperDestroyed(QObject*)));
    }
}

void KickerTip::untipFor(const QWidget* w)
{
    if (isTippingFor(w))
    {
        tipFor(0);
        d->timer.stop();
        hide();
    }
}

bool KickerTip::isTippingFor(const QWidget* w) const
{
    return d->tippingFor == w;
}

void KickerTip::tipperDestroyed(QObject* o)
{
    // we can't do a dynamic cast because we are in the process of dieing
    // so static it is.
    untipFor(static_cast<QWidget*>(o));
}

void KickerTip::internalUpdate()
{
    d->dirty = true;
    repaint();
}

void KickerTip::enableTipping(bool tip)
{
    if (tip)
    {
        m_tippingEnabled++;
    }
    else
    {
        m_tippingEnabled--;
    }

    if (m_tippingEnabled < 1 && m_self)
    {
        m_self->hide();
    }
}

bool KickerTip::tippingEnabled()
{
    return m_tippingEnabled > 0;
}

void KickerTip::hide()
{
    d->tippingFor = 0;
    QWidget::hide();
}

bool KickerTip::eventFilter(QObject *object, QEvent *event)
{
    if (!tippingEnabled())
    {
        return false;
    }

    if (!object->isWidgetType())
    {
        return false;
    }

    QWidget *widget = static_cast<QWidget*>(object);

    switch (event->type())
    {
        case QEvent::ToolTip:
            if (KickerSettings::showMouseOverEffects())
            {
                // Prevent Qt from showing a tooltip for the button
                event->accept();
                return true;
            }
            return false;

        case QEvent::Enter:
            if (!KickerSettings::showMouseOverEffects())
            {
                return false;
            }

            if (!mouseGrabber() &&
                !qApp->activePopupWidget() &&
                !isTippingFor(widget))
            {
                tipFor(widget);
                d->timer.stop();
                disconnect(&d->timer, SIGNAL(timeout()), 0, 0);
                connect(&d->timer, SIGNAL(timeout()), SLOT(display()));

                // delay to avoid false starts
                // e.g. when the user quickly zooms their mouse over
                // a button then out of kicker
                if (isVisible())
                {
                    d->timer.start(KickerSettings::mouseOversShowDelay() / 3);
                }
                else
                {
                    d->timer.start(KickerSettings::mouseOversShowDelay());
                }
            }
            break;
        case QEvent::Leave:
            d->timer.stop();

            if (isTippingFor(widget) && isVisible())
            {
                disconnect(&d->timer, SIGNAL(timeout()), 0, 0);
                connect(&d->timer, SIGNAL(timeout()), SLOT(hide()));
                d->timer.start(KickerSettings::mouseOversHideDelay());
            }

            tipFor(0);
            break;
        case QEvent::MouseButtonPress:
            d->timer.stop();
            d->frameTimer.stop();
            hide();
        default:
            break;
    }

    return false;
}

#include <kickertip.moc>

