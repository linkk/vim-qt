#include "mainwindow.moc"

MainWindow::MainWindow( gui_T* gui, QWidget *parent)
:QMainWindow(parent), m_keepTabbar(false), scrollArea(0)
{
	setWindowIcon(QIcon(":/icons/vim-qt.png"));
	setContextMenuPolicy(Qt::PreventContextMenu);

	// Menu
	menutoolbar = addToolBar("Menu");
	menutoolbar->setObjectName("menu");
	menutoolbar->setAllowedAreas(Qt::BottomToolBarArea | Qt::TopToolBarArea);
	menu = new QMenuBar(menutoolbar);
	menutoolbar->addWidget(menu);

	// Tool bar
	toolbar = addToolBar("ToolBar");
	toolbar->setObjectName("toolbar");

	scrollArea = new ScrollArea(this);
	setCentralWidget( scrollArea );

	// Vim shell
	vimshell = new QVimShell( scrollArea );
	vimshell->setFocus();
	scrollArea->setWidget(vimshell);

	// TabLine
	tabtoolbar = addToolBar("tabline");
	tabtoolbar->setObjectName("tabline");

	connect( tabtoolbar, SIGNAL(orientationChanged(Qt::Orientation)),
			this, SLOT(updateTabOrientation()) );
	connect( tabtoolbar, SIGNAL(topLevelChanged(bool)),
			this, SLOT(updateTabOrientation()));

	tabbar = new TabBar(tabtoolbar);
	tabbar->setTabsClosable(true);
	tabbar->setExpanding(false);
	tabbar->setFocusPolicy(Qt::NoFocus);
	tabbar->setDrawBase(false);
	tabbar->setMovable(true);
	connect(tabbar, SIGNAL(tabMoved(int, int)),
			this, SLOT(tabMoved(int, int)));


	tabbar->addTab("VIM"); // One tab must always exist

	tabtoolbar->addWidget(tabbar);
	QAction *newTab = tabtoolbar->addAction( VimWrapper::icon("tab-new"), "New Tab");

	connect( tabbar, SIGNAL(tabCloseRequested(int)),
			this, SLOT(closeTab(int)));
	connect( tabbar, SIGNAL(currentChanged(int)),
			this, SLOT(switchTab(int)));
	connect( newTab, SIGNAL(triggered()),
			this, SLOT(openNewTab()));
}

void MainWindow::tabMoved(int from, int to)
{
	//
	// 1. In some cases Qt will not drag the tab you expect
	// for example if you have two tabs (1,2) and drag 
	// tab 1 to place 2 -> Qt might trigger the reverse
	// event i.e. move tab 2 to place 1
	//
	// 2. It also seems that QTabbar::currentIndex() always 
	// returns the current tab, i.e. after the movement event
	// it returns the destination position - which is exactly
	// what we want.
	//
	// 3. Also, both tabpage_move and Qt use 0-indexed tab positions
	//

	qDebug() << __func__ << "(current: " << tabbar->currentIndex() << ") from: " << from << "to:" << to;
	tabpage_move(tabbar->currentIndex());
}

void MainWindow::updateTabOrientation()
{

	if ( tabtoolbar->orientation() == Qt::Horizontal ) {
		if ( toolBarArea(tabtoolbar) == Qt::BottomToolBarArea ) {
			tabbar->setShape( QTabBar::RoundedSouth );
		} else {
			tabbar->setShape( QTabBar::RoundedNorth );
		}
	} else {
		if ( toolBarArea(tabtoolbar) == Qt::LeftToolBarArea ) {
			tabbar->setShape( QTabBar::RoundedWest );
		} else {
			tabbar->setShape( QTabBar::RoundedEast );
		}
	}

	if ( tabtoolbar->isFloating() ) {
		tabtoolbar->resize(tabtoolbar->minimumWidth(), tabtoolbar->sizeHint().height());
	}
}

bool MainWindow::restoreState(const QByteArray& state, int version)
{
	bool ret = QMainWindow::restoreState(state, version);
	if ( keepTabbar() ) {
		showTabline(true);
	}

	return ret;
}

QVimShell* MainWindow::vimShell()
{
	return this->vimshell;
}

QMenuBar* MainWindow::menuBar() const
{
	return menu;
}

QToolBar* MainWindow::toolBar() const
{
	return toolbar;
}

void MainWindow::closeEvent (QCloseEvent * event)
{
	vimshell->closeEvent(event);
}

void MainWindow::showTabline(bool show)
{
	// VIM never removes the second tab,
	// instead it hides the entire tab bar
	if ( keepTabbar() && !show ) {
		removeTabs(1);
	}

	if ( keepTabbar() ) {
		tabtoolbar->setVisible(true);
	} else {
		tabtoolbar->setVisible(show);
	}
}

void MainWindow::showToolbar(bool show)
{
	toolbar->setVisible(show);
}

void MainWindow::showMenu(bool show)
{
	menutoolbar->setVisible(show);
}

bool MainWindow::tablineVisible()
{
	return tabtoolbar->isVisible();
}

void MainWindow::setCurrentTab(int idx)
{
	tabbar->setCurrentIndex(idx);
}

void MainWindow::setTab( int idx, const QString& label)
{
	while ( tabbar->count() <= idx ) {
		tabbar->addTab("[No name]");
	}

	tabbar->setTabText(idx, label);
}


void MainWindow::removeTabs(int idx)
{
	int i;
	for ( i=idx; i<tabbar->count(); i++) {
		tabbar->removeTab(i);
	}
}

void MainWindow::switchTab(int idx)
{
	vimshell->switchTab(idx+1);
}

void MainWindow::closeTab(int idx)
{
	if ( keepTabbar() && tabbar->count() == 1 ) {
		vimshell->close();
	} else {
		vimshell->closeTab(idx+1);
	}
}

void MainWindow::openNewTab()
{
	// The amusing +1 trick opens the
	// tab at the right of other tabs
	VimWrapper::newTab(tabbar->count()+1);
}

void MainWindow::setKeepTabbar(bool keep)
{
	m_keepTabbar = keep;
}

bool MainWindow::keepTabbar()
{
	return m_keepTabbar;
}

