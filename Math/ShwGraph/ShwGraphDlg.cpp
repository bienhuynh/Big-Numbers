#include "stdafx.h"
#include <process.h>
#include "ShwGraph.h"
#include "ShwGraphDlg.h"
#include "DegreeDlg.h"
#include <Math/MathFunctions.h>
#include <Math/MathException.h>
#include "IntervalDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialog {
public:
    CAboutDlg();

    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);

protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD) {
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

CShwGraphDlg::CShwGraphDlg(CWnd* pParent) : CDialog(CShwGraphDlg::IDD, pParent) {
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_preferredDegree = 0;
}

void CShwGraphDlg::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CShwGraphDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_SYSCOMMAND()
    ON_WM_SIZE()
    ON_WM_DROPFILES()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_MOUSEWHEEL()
    ON_COMMAND(ID_FILE_OPEN                   , OnFileOpen                   )
    ON_COMMAND(ID_FILE_QUIT                   , OnFileQuit                   )
    ON_COMMAND(ID_EDIT_DEGREE                 , OnEditDegree                 )
    ON_COMMAND(ID_EDIT_INCRDEGREE             , OnEditIncrdegree             )
    ON_COMMAND(ID_EDIT_DECRDEGREE             , OnEditDecrdegree             )
    ON_COMMAND(ID_EDIT_LEASTSUMOFSQUARES      , OnEditLeastSumOfSquares      )
    ON_COMMAND(ID_EDIT_CUBICSPLINE_NATURAL    , OnEditCubicSplineNatural     )
    ON_COMMAND(ID_EDIT_CUBICSPLINE_MATCHDYDX  , OnEditCubicSplineMatchdydx   )
    ON_COMMAND(ID_EDIT_CUBICSPLINE_NOENDPOINTS, OnEditCubicSplineINoEndPoints)
    ON_COMMAND(ID_EDIT_AKIMASPLINE            , OnEditAkimaSpline            )
    ON_COMMAND(ID_EDIT_FRITSCHCARLSONSPLINE   , OnEditFritschCarlsonSpline   )
    ON_COMMAND(ID_EDIT_CLEAR                  , OnEditClear                  )
    ON_COMMAND(ID_TOOLS_SHOWFUNCTION          , OnToolsShowFunction          )
    ON_COMMAND(ID_TOOLS_SHOWD1                , OnToolsShowD1                )
    ON_COMMAND(ID_TOOLS_SHOWD2                , OnToolsShowD2                )
    ON_COMMAND(ID_TOOLS_SHOWD3                , OnToolsShowD3                )
    ON_COMMAND(ID_TOOLS_SHOWMOUSEPOSITION     , OnToolsShowMousePosition     )
    ON_COMMAND(ID_TOOLS_ZOOMIN                , OnToolsZoomin                )
    ON_COMMAND(ID_TOOLS_ZOOMOUT               , OnToolsZoomout               )
    ON_COMMAND(ID_TOOLS_SETINTERVAL           , OnToolsSetinterval           )
    ON_COMMAND(ID_TOOLS_GRID                  , OnToolsGrid                  )
    ON_COMMAND(ID_TOOLS_FONT                  , OnToolsFont                  )
    ON_COMMAND(ID_TOOLS_GENERATECFUNCTION     , OnToolsGeneratecfunction     )
    ON_COMMAND(ID_TOOLS_DISPLAYCOEFFICIENTS   , OnToolsDisplaycoefficients   )
    ON_COMMAND(ID_HELP_ABOUT                  , OnHelpAbout                  )
END_MESSAGE_MAP()

BOOL CShwGraphDlg::OnInitDialog() {
  CDialog::OnInitDialog();

  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu != NULL) {
      CString strAboutMenu;
      strAboutMenu.LoadString(IDS_ABOUTBOX);
      if (!strAboutMenu.IsEmpty()) {
          pSysMenu->AppendMenu(MF_SEPARATOR);
          pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
      }
  }

  m_currentFont = NULL;
  m_defaultFont.CreateFont(  8, 8, 0, 0, 400, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                          DEFAULT_PITCH | FF_MODERN,
                          _T("Courier") );
  setCurrentFont(&m_defaultFont);

  m_cs.substituteControl(this, IDC_COORSYSTEM);
  m_layout.OnInitDialog(this);
  m_layout.addControl(IDC_COORSYSTEM, RELATIVE_SIZE);
  m_layout.addControl(IDC_STATICINFO, RELATIVE_Y_POS | RELATIVE_WIDTH);

  SetIcon(m_hIcon, TRUE); 
  SetIcon(m_hIcon, FALSE);

  m_needSolve  = true;
  m_canDraw    = false;
  setInterpolType(ID_EDIT_LEASTSUMOFSQUARES);
  setState(STATE_IDLE);

  m_accelTable   = LoadAccelerators(AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));
  m_systemCursor = CopyCursor(AfxGetApp()->LoadOEMCursor(OCR_NORMAL));

  m_whiteBrush.CreateSolidBrush(RGB(255,255,255));
  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CShwGraphDlg::showException(const Exception &e) {
  MessageBox(e.what(), _T("Error"), MB_ICONWARNING);
}

void CShwGraphDlg::OnSysCommand(UINT nID, LPARAM lParam) {
  if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
  } else {
    CDialog::OnSysCommand(nID, lParam);
  }
}

void CShwGraphDlg::OnPaint() {
  if (IsIconic()) {
    CPaintDC dc(this);

    SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else {
    CPaintDC dc(&m_cs);
    m_cs.paint(dc);
    m_cs.setDC(dc);
    Viewport2D &vp = m_cs.getViewport();
    vp.setClipping(true);
    for(size_t i = 0; i < m_points.size(); i++) {
      vp.paintCross(m_points[i],BLACK);
    }

    solve();

    m_dataFit.setDegree(m_preferredDegree);

    try {
      if(showFunctionIsChecked()) drawDerived(vp,0);
      if(showD1IsChecked())       drawDerived(vp,1);
      if(showD2IsChecked())       drawDerived(vp,2);
      if(showD3IsChecked())       drawDerived(vp,3);
      if(showCoefIsChecked())     showCoef();
    } catch(Exception e) {
      m_canDraw = false;
      showException(e);
    }

  }
  CDialog::OnPaint();
}

HCURSOR CShwGraphDlg::OnQueryDragIcon() {
  return (HCURSOR) m_hIcon;
}

void CShwGraphDlg::OnSize(UINT nType, int cx, int cy) {
  CDialog::OnSize(nType, cx, cy);
  m_layout.OnSize(nType, cx, cy);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnDropFiles(HDROP hDropInfo) {
  TCHAR fname[256];
  int count = DragQueryFile(hDropInfo,-1,fname,ARRAYSIZE(fname));

  DragQueryFile(hDropInfo,0,fname,ARRAYSIZE(fname));
  readFile(fname);
  CDialog::OnDropFiles(hDropInfo);
}

void CShwGraphDlg::OnOK() {
}

void CShwGraphDlg::OnCancel() {
  exit(0);
}

bool CShwGraphDlg::showMousePositionIsChecked() { return isMenuItemChecked(this, ID_TOOLS_SHOWMOUSEPOSITION  ); }
bool CShwGraphDlg::showFunctionIsChecked()      { return isMenuItemChecked(this, ID_TOOLS_SHOWFUNCTION       ); }
bool CShwGraphDlg::showD1IsChecked()            { return isMenuItemChecked(this, ID_TOOLS_SHOWD1             ); }
bool CShwGraphDlg::showD2IsChecked()            { return isMenuItemChecked(this, ID_TOOLS_SHOWD2             ); }
bool CShwGraphDlg::showD3IsChecked()            { return isMenuItemChecked(this, ID_TOOLS_SHOWD3             ); }
bool CShwGraphDlg::gridIsChecked()              { return isMenuItemChecked(this, ID_TOOLS_GRID               ); }
bool CShwGraphDlg::showCoefIsChecked()          { return isMenuItemChecked(this, ID_TOOLS_DISPLAYCOEFFICIENTS); }
bool CShwGraphDlg::autoUpdateScreen() {
  return showCoefIsChecked() || showFunctionIsChecked() || showD1IsChecked() || showD2IsChecked() || showD3IsChecked();
}

void CShwGraphDlg::setState(DialogState newstate) {
  m_state = newstate;
}

void CShwGraphDlg::addPoint(const Point2DP &p) {
  m_points.add(p);
  m_needSolve = true;
}

void CShwGraphDlg::removePoint(intptr_t index) {
  m_points.remove(index);
  m_needSolve = true;
}

void CShwGraphDlg::movePoint(intptr_t index, const CPoint &point) {
  CClientDC dc(&m_cs);
  m_cs.setDC(dc);
  Viewport2D &vp = m_cs.getViewport();
  vp.paintCross(m_points[index],WHITE);
  m_points[index] = getTr().backwardTransform(Point2DP(point));
  vp.paintCross(m_points[index],BLACK);
  m_needSolve = true;
  if(autoUpdateScreen()) {
    Invalidate(FALSE);
  }
}

void CShwGraphDlg::readData(FILE *f) {
  TCHAR line[100];
  m_points.clear();
  while(FGETS(line,ARRAYSIZE(line),f)) {
    Point2D p;
    if(_stscanf(line,_T("%le %le"), &p.x, &p.y) != 2)
      continue;
    addPoint(p);
  }
}

void CShwGraphDlg::adjustTransform() {
  double minx,maxx,miny,maxy;
  if(m_points.size()) {
    minx = maxx = m_points[0].x;
    miny = maxy = m_points[0].y;
    for(unsigned int i = 1;i < m_points.size();i++) {
      if(m_points[i].x < minx) minx = m_points[i].x;
      if(m_points[i].x > maxx) maxx = m_points[i].x;
      if(m_points[i].y < miny) miny = m_points[i].y;
      if(m_points[i].y > maxy) maxy = m_points[i].y;
    }
    if(minx == maxx) { minx--; maxx++; }
    if(miny == maxy) { miny--; maxy++; }
    m_cs.setDataRange(Rectangle2DR(minx,maxx,miny,maxy), true);
  }
}

intptr_t CShwGraphDlg::findDataPoint(const CPoint &point) { /* returns the index of the nearest DataPoint. -1 if none */
  double mindist = 100;
  intptr_t found = -1;
  RectangleTransformation &tr = getTr();
  for(size_t i = 0; i < m_points.size();i++) {
    Point2DP pp = tr.forwardTransform(m_points[i]);
    if(abs(pp.x - point.x) > 5 || abs(pp.y - point.y) > 5)
      continue;
    double dist = sqr(pp.x - point.x)+sqr(pp.y-point.y);
    if(dist < 25 && dist < mindist) {
      mindist = dist;
      found = i;
    }
  }
  return found;
}

void CShwGraphDlg::readFile(const String &fname) {
  FILE *f = fopen(fname, _T("r"));
  if(f) {
    readData(f);
    fclose(f);
    if(m_points.size()) {
      adjustTransform();
    }
    Invalidate(FALSE);
  } else {
    MessageBox(format(_T("Could not open %s"),fname.cstr()).cstr(), _T("Error"), MB_ICONWARNING);
  }
}

void CShwGraphDlg::OnFileOpen() {
  CFileDialog dlg(TRUE);
  if(dlg.DoModal() != IDOK) return;
  if(_tcslen(dlg.m_ofn.lpstrFile) == 0) {
    return;
  }
  readFile(dlg.m_ofn.lpstrFile);
}

void CShwGraphDlg::OnFileQuit() {
  exit(0);  
}

void CShwGraphDlg::OnEditDegree() {
  DegreeDlg dlg(m_preferredDegree);
  if(dlg.DoModal() == IDOK) {
    m_preferredDegree = dlg.m_degree;
    if(autoUpdateScreen()) {
      Invalidate(FALSE);
    }
  }
}

void CShwGraphDlg::OnEditClear() {
  m_points.clear();
  m_needSolve = true;
  Invalidate(FALSE);
}

void CShwGraphDlg::OnToolsShowFunction() {
  toggleMenuItem(this, ID_TOOLS_SHOWFUNCTION);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnToolsShowD1() {
  toggleMenuItem(this, ID_TOOLS_SHOWD1);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnToolsShowD2() {
  toggleMenuItem(this, ID_TOOLS_SHOWD2);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnToolsShowD3() {
  toggleMenuItem(this, ID_TOOLS_SHOWD3);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnToolsDisplaycoefficients() {
  toggleMenuItem(this, ID_TOOLS_DISPLAYCOEFFICIENTS);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnToolsShowMousePosition() {
  toggleMenuItem(this, ID_TOOLS_SHOWMOUSEPOSITION);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnToolsZoomin() {
  if(m_state == STATE_IDLE) {
    setState(STATE_ZOOM_START);
    setWindowCursor(this,IDC_CURSORZOOM);
  }
}

void CShwGraphDlg::OnToolsZoomout() { 
  if(m_zoomStack.isEmpty()) {
    return;
  }
  popZoom();
  Invalidate(FALSE);
}

DoubleInterval CShwGraphDlg::getXInterval() {
  if(uselssd()) {
    return getTr().getXTransformation().getFromInterval();
  } else {
    return DoubleInterval(minXValue(),maxXValue());
  }
}

DoubleInterval CShwGraphDlg::findYInterval(const DoubleInterval &xInterval) {
  solve();
  if(!m_canDraw) {
    return DoubleInterval(-10,10);
  }
  else {
    double step = (xInterval.getTo() - xInterval.getFrom()) / 200;
    double x = xInterval.getFrom();
    double miny,maxy;
    for(int i = 0; i < 200; i++, x+=step) {
      double y = evaluate(x);
      if(i == 0) {
        maxy = miny = y;
      }
      else {
        miny = min(y,miny);
        maxy = max(y,maxy);
      }
    }
    return DoubleInterval(miny,maxy);
  }
}

void CShwGraphDlg::OnToolsSetinterval() {
  IntervalDlg dlg;
  RectangleTransformation &tr = getTr();
  dlg.m_minx = tr.getFromRectangle().getMinX();
  dlg.m_maxx = tr.getFromRectangle().getMaxX();
  dlg.m_miny = tr.getFromRectangle().getMinY();
  dlg.m_maxy = tr.getFromRectangle().getMaxY();

  if(dlg.DoModal() == IDOK) {
    DoubleInterval xInterval(dlg.m_minx, dlg.m_maxx);
    DoubleInterval yInterval(dlg.m_miny, dlg.m_maxy);
    if(dlg.m_autoscaley) {
      yInterval = findYInterval(xInterval);
    }
    tr.setFromRectangle(Rectangle2DR(xInterval.getFrom(),yInterval.getTo(),xInterval.getTo(), yInterval.getFrom()));
    Invalidate(FALSE);
  }
}

void CShwGraphDlg::OnToolsGrid() {
  m_cs.setGrid(toggleMenuItem(this, ID_TOOLS_GRID));
  Invalidate(FALSE);
}

CPoint CShwGraphDlg::mouseToCS(const CPoint &p) const {
  CPoint result = p;
  ClientToScreen(&result);
  m_cs.ScreenToClient(&result);
  return result;
}

void CShwGraphDlg::OnLButtonDown(UINT nFlags, CPoint point) {
  point = mouseToCS(point);
  switch(m_state) {
  case STATE_IDLE:
    { m_movePoint = findDataPoint(point);
      setState(STATE_LBUTTONDOWN);
    }
    break;
  case STATE_ZOOM_START:
    { CClientDC dc(&m_cs);
      SIZE      ss;
      ss.cx = ss.cy = 1;
      m_zoomRect.right = m_zoomRect.left   = point.x;
      m_zoomRect.top   = m_zoomRect.bottom = point.y;
      dc.DrawDragRect(&m_zoomRect,ss,NULL,ss);
      setState(STATE_ZOOM_RESIZE);
    }
    break;
  }
  CDialog::OnLButtonDown(nFlags, point);
}

void CShwGraphDlg::showMousePosition(const CPoint &p) {
  if(!showMousePositionIsChecked()) return;
  const Point2D dp  = getTr().backwardTransform(Point2DP(p));
  const String  str = format(_T("(%+.2lf,%+.2lf)  "),dp.x,dp.y);
  setWindowText(this, IDC_STATICINFO, str);
}

void CShwGraphDlg::OnMouseMove(UINT nFlags, CPoint point) {
  point = mouseToCS(point);
  switch(m_state) {
  case STATE_LBUTTONDOWN:
    if(m_movePoint >= 0) {
      setState(STATE_MOVEPOINT);
      movePoint(m_movePoint, point);
    } else {
      CClientDC dc(&m_cs);
      m_cs.setDC(dc);
      Viewport2D &vp = m_cs.getViewport();
      addPoint(getTr().backwardTransform(Point2DP(point)));
      vp.paintCross(m_points.last(),BLACK);
      m_movePoint = m_points.size()-1;
      setState(STATE_MOVEPOINT);
      if(autoUpdateScreen()) Invalidate(FALSE);
    }
    break;
  case STATE_MOVEPOINT:
    movePoint(m_movePoint, point);
    break;
  case STATE_ZOOM_RESIZE: 
    { CClientDC dc(&m_cs);
      CRect     newRect;
      SIZE      ss;
      newRect        = m_zoomRect;
      newRect.right  = point.x;
      newRect.bottom = point.y;
      ss.cx = ss.cy = 1;
      dc.DrawDragRect(&newRect,ss,&m_zoomRect,ss);
      m_zoomRect = newRect;
    }
    break;
  }
  showMousePosition(point);
  CDialog::OnMouseMove(nFlags, point);
}

void CShwGraphDlg::OnLButtonUp(UINT nFlags, CPoint point) {
  point = mouseToCS(point);
  switch(m_state) {
  case STATE_LBUTTONDOWN:
    { CClientDC dc(&m_cs);
      m_cs.setDC(dc);
      Viewport2D &vp = m_cs.getViewport();
      addPoint(vp.backwardTransform(point));
      vp.paintCross(m_points.last(),BLACK);
      if(autoUpdateScreen()) Invalidate(FALSE);
      setState(STATE_IDLE);
    }
    break;
  case STATE_ZOOM_RESIZE:
    { CClientDC dc(&m_cs);
      SIZE      ss,oss;
      ss.cx  = ss.cy = 0;
      oss.cx = 1; oss.cy = 1;
      dc.DrawDragRect(&m_zoomRect,ss,&m_zoomRect,oss);
      pushZoom(m_zoomRect);
      setState(STATE_IDLE);
      setWindowCursor(this,IDC_ARROW);
      Invalidate(FALSE);
    }
    break;
  default:
    setState(STATE_IDLE);
    break;
  }
  CDialog::OnLButtonUp(nFlags, point);
}

void CShwGraphDlg::OnRButtonDown(UINT nFlags, CPoint point) {
  point = mouseToCS(point);
  switch(m_state) {
  case STATE_IDLE:
    { const intptr_t index = findDataPoint(point);
      if(index >= 0) {
        CClientDC dc(&m_cs);
        m_cs.setDC(dc);
        Viewport2D &vp = m_cs.getViewport();
        vp.paintCross(m_points[index],WHITE);
        removePoint(index);
        if(autoUpdateScreen()) Invalidate(FALSE);
      }
      break;
    }
  }
  CDialog::OnRButtonDown(nFlags, point);
}

void CShwGraphDlg::pushZoom(CRect &rect) {
  Point2DP tmpp;

  tmpp.x = rect.left;  tmpp.y = rect.top;
  Point2D dp1 = getTr().backwardTransform(tmpp);
  tmpp.x = rect.right; tmpp.y = rect.bottom;
  Point2D dp2 = getTr().backwardTransform(tmpp);
  if(dp1.x == dp2.x || dp1.y == dp2.y) {
    return;
  }
  if(dp1.x > dp2.x) ::swap(dp1.x,dp2.x);
  if(dp1.y > dp2.y) ::swap(dp1.y,dp2.y);

  pushZoom(dp1.x,dp2.x,dp1.y,dp2.y);
}

void CShwGraphDlg::pushZoom(double minx, double maxx, double miny, double maxy) {
  m_zoomStack.push(m_cs.getDataRange());
  m_cs.setDataRange(Rectangle2DR(minx,maxy,maxx,miny), true);
  enableMenuItem(this, ID_TOOLS_ZOOMOUT, true);
}

void CShwGraphDlg::popZoom() {
  if(m_zoomStack.isEmpty()) {
    return;
  }
  m_cs.setDataRange(m_zoomStack.pop(), true);
  if(m_zoomStack.isEmpty()) {
    enableMenuItem(this, ID_TOOLS_ZOOMOUT, false);
  }
}

static DataPoint convertPoint(const Point2DP &p) {
  return DataPoint(p.x,p.y,1);
}

static DataPointArray convertData(const Point2DArray &data) {
  DataPointArray result(data.size());
  for(size_t i = 0; i < data.size(); i++) {
    result.add(convertPoint(data[i]));
  }
  return result;
}

void CShwGraphDlg::solve() {
  try {
    if(m_needSolve) {
      m_dataPoints = convertData(m_points);
      if(m_dataFit.solve(getFittingType(),m_dataPoints)) {
        m_needSolve = false;
        m_canDraw   = true;
      } else {
        m_canDraw   = false;
      }
    }
  } catch(MathException e) {
    showException(e);
  }
}

double CShwGraphDlg::minXValue() const {
  return m_points.isEmpty() ? 0 : m_points.getBoundingBox().getMinX();
}

double CShwGraphDlg::maxXValue() const {
  return m_points.isEmpty() ? 0 : m_points.getBoundingBox().getMaxX();
}

double CShwGraphDlg::evaluate(double x) {
  return m_dataFit(x);
}

void CShwGraphDlg::drawFunction(Viewport2D &vp, Function &f) {
  DoubleInterval xInterval = getXInterval();
  DoubleInterval interval = vp.getXTransformation().forwardTransform(xInterval);
  if(interval.getFrom() == interval.getTo()) {
    return;
  }

  bool firstTime = true;
  for(double x = interval.getFrom(); x < interval.getTo(); x++) {
    double xx = firstTime ? xInterval.getFrom() : vp.getXTransformation().backwardTransform(x);
    double yy = f(xx);
    if(firstTime) {
      vp.MoveTo(Point2D(xx,yy));
      firstTime = false;
    } else {
      vp.LineTo(Point2D(xx,yy));
    }
  }
  vp.LineTo(Point2D(xInterval.getTo(),f(xInterval.getTo())));
}

class FunctionDerived : public Function {
private:
  int      m_d;
  DataFit &m_f;
public:
  FunctionDerived(DataFit &f, int d);
  double operator()(const Real &x);
};

FunctionDerived::FunctionDerived(DataFit &f, int d) : m_f(f) {
  m_d = d;
}

double FunctionDerived::operator()(const Real &x) { 
  return m_f(m_d,x);
}

void CShwGraphDlg::drawDerived(Viewport2D &vp, int d) {
  if(!m_canDraw) {
    return;
  }
  drawFunction(vp,FunctionDerived(m_dataFit,d));
}

void CShwGraphDlg::showCoef() {
  CClientDC dc(&m_cs);
  if(!uselssd() || !m_canDraw) {
    return;
  }

  const int fitDegree  = m_dataFit.getActualDegree();
  Polynomial ptmp(m_dataFit);
  const int polyDegree = ptmp.getDegree();
  const int degree     = min(fitDegree, polyDegree);

  CFont *oldFont = dc.SelectObject(m_currentFont);
  int i;
  for(i = 0; i <= degree; i++) {
    textOut(dc,1,15+i*15,format(_T("A[%2d]=%lg"),i,ptmp.getCoef(i).re));
  }
  double ssd = m_dataFit.ssd(m_dataPoints);
  textOut(dc, 1,15+(i++)*15,format(_T("SSD = %lg"),ssd));
  textOut(dc, 1,15+i*15,format(_T("S2=%lg"),ssd / (m_points.size() - degree - 1)));

  dc.SelectObject(oldFont);
}

void CShwGraphDlg::OnHelpAbout() {
  CAboutDlg dlg;
  dlg.DoModal();
}

BOOL CShwGraphDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
  ScreenToClient(&pt);
  int zoomFlags = 0;
  if(nFlags & MK_CONTROL)   zoomFlags |= X_AXIS;
  if(nFlags & MK_SHIFT)     zoomFlags |= Y_AXIS;
  if(zoomFlags == 0)        zoomFlags = X_AXIS | Y_AXIS;
  getTr().zoom(Point2DP(pt), double(zDelta) / 1000.0,zoomFlags);
  Invalidate(FALSE);
  return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CShwGraphDlg::OnEditIncrdegree() {
  if(m_preferredDegree >= MAXDEGREE) {
    return;
  }
  m_preferredDegree++;
  if(autoUpdateScreen()) {
    Invalidate(FALSE);
  }
}

void CShwGraphDlg::OnEditDecrdegree() {
  if(m_preferredDegree <= 0) {
    return;
  }
  m_preferredDegree--;
  if(autoUpdateScreen()) {
    Invalidate(FALSE);
  }
}

BOOL CShwGraphDlg::PreTranslateMessage(MSG* pMsg) {
  if(TranslateAccelerator(m_hWnd,m_accelTable,pMsg)) {
    return true;
  }
  return CDialog::PreTranslateMessage(pMsg);
}

void CShwGraphDlg::setChecked(int id1, int id2) {
  checkMenuItem(this, id1,id1==id2);
}

FittingType CShwGraphDlg::getFittingType() {
  switch(m_iptype) {
  case ID_EDIT_LEASTSUMOFSQUARES      : return LSSD;
  case ID_EDIT_CUBICSPLINE_NATURAL    : return NATURAL_SPLINE;
  case ID_EDIT_CUBICSPLINE_MATCHDYDX  : return DYDXMATCH_SPLINE;
  case ID_EDIT_CUBICSPLINE_NOENDPOINTS: return IGNOREENDPOINT_SPLINE;
  case ID_EDIT_AKIMASPLINE            : return AKIMA_SPLINE;
  case ID_EDIT_FRITSCHCARLSONSPLINE   : return FRITSCH_CARLSON;
  }
  return LSSD;
}

void CShwGraphDlg::setInterpolType(int id) {
  setChecked(ID_EDIT_LEASTSUMOFSQUARES           ,id);
  setChecked(ID_EDIT_CUBICSPLINE_NATURAL         ,id);
  setChecked(ID_EDIT_CUBICSPLINE_MATCHDYDX       ,id);
  setChecked(ID_EDIT_CUBICSPLINE_NOENDPOINTS     ,id);
  setChecked(ID_EDIT_AKIMASPLINE                 ,id);
  setChecked(ID_EDIT_FRITSCHCARLSONSPLINE        ,id);
  m_needSolve = true;
  m_iptype    = id;
}

void CShwGraphDlg::OnEditLeastSumOfSquares() {
  setInterpolType(ID_EDIT_LEASTSUMOFSQUARES);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnEditCubicSplineNatural() {
  setInterpolType(ID_EDIT_CUBICSPLINE_NATURAL);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnEditCubicSplineMatchdydx() {
  setInterpolType(ID_EDIT_CUBICSPLINE_MATCHDYDX);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnEditCubicSplineINoEndPoints() {
  setInterpolType(ID_EDIT_CUBICSPLINE_NOENDPOINTS);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnEditAkimaSpline() {
  setInterpolType(ID_EDIT_AKIMASPLINE);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnEditFritschCarlsonSpline() {
  setInterpolType(ID_EDIT_FRITSCHCARLSONSPLINE);
  Invalidate(FALSE);
}

static void showfile(const TCHAR *fname) {
  USES_CONVERSION;
  const char *aname = T2A(fname);
  if(_spawnl(_P_NOWAIT,"c:\\windows\\system32\\notepad.exe","notepad.exe",aname, NULL ) < 0) {
    AfxMessageBox(_T("Kan ikke starte notepad"), MB_ICONEXCLAMATION);
  }
}

void CShwGraphDlg::OnToolsGeneratecfunction() {
  if(!m_canDraw) {
    return;
  }

  TCHAR *fname = _T("c:\\temp\\koef.txt");
  FILE  *f     = fopen(fname,_T("w"));
  if(f) {
    _ftprintf(f,_T("%s"),m_dataFit.generateCFunction().cstr());
    fclose(f);
    showfile(fname);
    Invalidate();
  }
}

void CShwGraphDlg::setCurrentFont(CFont *newfont) {
  if(m_currentFont && m_currentFont != &m_defaultFont) {
    m_currentFont->DeleteObject();
    delete m_currentFont;
  }

  m_currentFont = newfont;
  GetDlgItem(IDC_STATICINFO)->SetFont(m_currentFont);
  Invalidate(FALSE);
}

void CShwGraphDlg::OnToolsFont() {
  LOGFONT curlf;
  m_currentFont->GetLogFont(&curlf);
  CFontDialog  dlg(&curlf);
  if(dlg.DoModal() == IDOK) {
    LOGFONT lf;
    dlg.GetCurrentFont(&lf);
    CFont *newfont = new CFont;
    newfont->CreateFontIndirect(&lf);

    setCurrentFont(newfont);
  }
}
