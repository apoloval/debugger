// $Id$

#include "StackViewer.h"
#include "OpenMSXConnection.h"
#include "CommClient.h"
#include "Settings.h"
#include <QScrollBar>
#include <QPaintEvent>
#include <QPainter>
#include <cmath>

class StackRequest : public ReadDebugBlockCommand
{
public:
	StackRequest(unsigned offset_, unsigned size,
	           unsigned char* target, StackViewer& viewer_)
		: ReadDebugBlockCommand("memory", offset_, size, target)
		, offset(offset_)
		, viewer(viewer_)
	{
	}

	virtual void replyOk(const QString& message)
	{
		copyData(message);
		viewer.memdataTransfered(this);
	}

	virtual void cancel()
	{
		viewer.transferCancelled(this);
	}

	// TODO public members are ugly!!
	unsigned offset;

private:
	StackViewer& viewer;
};


StackViewer::StackViewer(QWidget* parent)
	: QFrame(parent)
{
	setFrameStyle(WinPanel | Sunken);
	setFocusPolicy(Qt::StrongFocus);
	setBackgroundRole(QPalette::Base);
	setSizePolicy(QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred));

	setFont(Settings::get().font(Settings::HEX_FONT));

	stackPointer = 0;
	topAddress = 0;
	waitingForData = false;

	vertScrollBar = new QScrollBar(Qt::Vertical, this);
	vertScrollBar->hide();

	frameL = frameT = frameB = frameWidth();
	frameR = frameL + vertScrollBar->sizeHint().width();

	setMinimumHeight(frameT+frameB+fontMetrics().height());

	connect(vertScrollBar, SIGNAL(valueChanged(int)),
	        this, SLOT(setLocation(int)));
}

QSize StackViewer::sizeHint() const
{
	return QSize(frameL + 4 + fontMetrics().width("FFFFWFFFF ") + 4 + frameR,
	             frameT + 8 * fontMetrics().height() + frameB);
}

void StackViewer::setScrollBarValues()
{
	vertScrollBar->setMinimum(stackPointer);

	visibleLines = double(height() - frameT - frameB) / fontMetrics().height();

	int lines = (memoryLength - stackPointer) / 2;
	vertScrollBar->setMaximum(stackPointer + 2 * (lines - int(visibleLines)));
	vertScrollBar->setSingleStep(2);
	vertScrollBar->setPageStep(2 * int(visibleLines));
}

void StackViewer::resizeEvent(QResizeEvent* e)
{
	QFrame::resizeEvent(e);

	setScrollBarValues();
	vertScrollBar->setGeometry(width() - frameR, frameT,
	                           vertScrollBar->sizeHint().width(),
	                           height() - frameT - frameB);
	vertScrollBar->show();
	// calc the number of lines that can be displayed
	// partial lines count as a whole
}

void StackViewer::paintEvent(QPaintEvent* e)
{
	// call parent for drawing the actual frame
	QFrame::paintEvent(e);

	QPainter p(this);
	int h = fontMetrics().height();
	int d = fontMetrics().descent();

	// set font
	p.setPen(Qt::black);

	// calc and set drawing bounds
	QRect r(e->rect());
	if (r.left() < frameL) r.setLeft(frameL);
	if (r.top()  < frameT) r.setTop (frameT);
	if (r.right()  > width()  - frameR - 1) r.setRight (width()  - frameR - 1);
	if (r.bottom() > height() - frameB - 1) r.setBottom(height() - frameB - 1);
	p.setClipRect(r);

	// redraw background
	p.fillRect(r, palette().color(QPalette::Base));

	// calc layout (not optimal)
	int xAddr = frameL + 8;
	int xStack = xAddr + fontMetrics().width("FFFFF");
	int y = frameT + h - 1;
	int address = topAddress;

	for (int i = 0; i < int(ceil(visibleLines)); ++i) {
		// print address
		QString hexStr;
		hexStr.sprintf("%04X", address);
		p.drawText(xAddr,  y - d, hexStr);
		hexStr.sprintf("%02X%02X", memory[address + 1], memory[address]);
		p.drawText(xStack, y - d, hexStr);
		y += h;
		address += 2;
		if (address >= memoryLength - 1) break;
	}
}

void StackViewer::setData(unsigned char* memPtr, int memLength)
{
	memory = memPtr;
	memoryLength = memLength;
	setScrollBarValues();
}

void StackViewer::setLocation(int addr)
{
	if (waitingForData) {
		// ignore
		return;
	}
	int start = (addr & ~1) | (stackPointer & 1);
	int size = 2 * int(ceil(visibleLines));

	if (start + size >= memoryLength) {
		size = memoryLength - start;
	}
	StackRequest* req = new StackRequest(start, size,
	                                     &memory[start], *this);
	CommClient::instance().sendCommand(req);
	waitingForData = true;
}

void StackViewer::setStackPointer(quint16 addr)
{
	stackPointer = addr;
	setScrollBarValues();
	vertScrollBar->setValue(addr);
	setLocation(addr);
}

void StackViewer::memdataTransfered(StackRequest* r)
{
	topAddress = r->offset;
	update();

	waitingForData = false;
	delete r;

	// check whether a new value is available
	if ((topAddress & ~1) != (vertScrollBar->value() & ~1)) {
		setLocation(vertScrollBar->value());
	}
}

void StackViewer::transferCancelled(StackRequest* r)
{
	waitingForData = false;
	delete r;
}
