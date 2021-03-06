// $Id$

#ifndef HEXVIEWER_H
#define HEXVIEWER_H

#include <QFrame>

class HexRequest;
class QScrollBar;
class QPaintEvent;

class HexViewer : public QFrame
{
	Q_OBJECT
public:
	HexViewer(QWidget* parent = 0);
	~HexViewer();

	enum Mode { FIXED, FILL_WIDTH, FILL_WIDTH_POWEROF2 };

	void setDebuggable(const QString& name, int size);
	void setIsInteractive(bool enabled);
	void setUseMarker(bool enabled);
	void setIsEditable(bool enabled);

	void setDisplayMode(Mode mode);
	void setDisplayWidth(short width);

	QSize sizeHint() const;

public slots:
	void setLocation(int addr);
	void setTopLocation(int addr);
	void scrollBarChanged(int addr);
	void settingsChanged();
	void refresh();

private:
	void wheelEvent(QWheelEvent* e);
	void resizeEvent(QResizeEvent* e);
	void paintEvent(QPaintEvent* e);
	void mousePressEvent(QMouseEvent* e);
	bool event(QEvent* e);
	void keyPressEvent(QKeyEvent* e);
	void focusInEvent(QFocusEvent* e);
	void focusOutEvent(QFocusEvent* e);

	void createActions();

	void setSizes();
	void hexdataTransfered(HexRequest* r);
	void transferCancelled(HexRequest* r);
	int coorToOffset(int x, int y);

	QScrollBar* vertScrollBar;
	QAction *fillWidthAction;
	QAction *fillWidth2Action;
	QAction *setWith8Action;
	QAction *setWith16Action; 
	QAction *setWith32Action;

	// layout
	int frameL, frameR, frameT, frameB;
	int leftCharPos, leftValuePos, rightValuePos, rightCharPos;
	Mode displayMode;
	short horBytes;
	int visibleLines, partialBottomLine;
	int lineHeight, xAddr, xData, xChar, dataWidth, charWidth, hexCharWidth;
	int addressLength;

	// data
	QString debuggableName;
	int debuggableSize;
	int hexTopAddress;
	int hexMarkAddress;
	unsigned char* hexData;
	unsigned char* previousHexData;
	bool waitingForData;
	bool highlitChanges;
	bool useMarker;
	bool isInteractive;
	bool isEditable;
	bool beingEdited;
	bool editedChars;
	bool hasFocus;
	int cursorPosition,editValue;

	friend class HexRequest;

private slots:
	void changeWidth();

signals:
	void locationChanged(int addr);
};

#endif // HEXVIEWER_H
