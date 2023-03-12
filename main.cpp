#include <math.h>
#include <fmt/core.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

#include <wx/wxprec.h> 
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif


// Функции для проверки клика в треугольник
float sign (wxPoint p1, wxPoint p2, wxPoint p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
};

bool pointInTriangle (wxPoint pt, wxPoint v1, wxPoint v2, wxPoint v3)
{
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
};

// Исключение для ошибок во время сохранения
class SaveException
{
private:
    std::string m_error;

public:
    SaveException(std::string error)
        : m_error{ fmt::format("Ошибка во время сохранения файла: {}", error) }
    {
    }

    std::string getError() const { return m_error; }
};

// Исключение для ошибок во время загрузки
class LoadException
{
private:
    std::string m_error;

public:
    LoadException(std::string error)
        : m_error{ fmt::format("Ошибка во время загрузки файла: {}", error) }
    {
    }

    std::string getError() const { return m_error; }
};

// Исключение для ошибок при загрузке типа фигуры
class WrongFigureTypeException
{
private:
    std::string m_error;

public:
    WrongFigureTypeException(std::string type)
        : m_error{ fmt::format("Неверный тип фигуры: {}", type) }
    {
    }

    std::string getError() const { return m_error; }
};

// Исключение для ошибок при загрузке сторон треугольника
class WrongTriangleSizeException
{
private:
    std::string m_error;

public:
    WrongTriangleSizeException(float a, float b, float c)
        : m_error{ fmt::format("Неверные размеры сторон для треугольника: {:.2f}, {:.2f}, {:.2f}", a, b, c) }
    {
    }

    std::string getError() const { return m_error; }
};

// Кисть для закраски фигур
wxBrush* brush = new wxBrush(*(new wxColour((unsigned long)rand())));

// Класс для фигур
class Figure
{
private:
    int _x, _y, _z;
    unsigned long _color;
public:
    Figure(int x, int y) {
        _x = x;
        _y = y;
        _z = 0;
    };
    Figure(int x, int y, unsigned long color): Figure(x,y) {
        _color = color;
    };
    Figure(Figure &copy): Figure(copy.GetX(), copy.GetY(), copy.GetColour()) {
        _z = copy.GetZ();
    };
    auto operator<=>(Figure* other) {
        if (CalcArea() < other->CalcArea()) return -1;
        if (CalcArea() > other->CalcArea()) return 1;
        return 0;
    };
    bool operator>(Figure* other) {
        return CalcArea() > other->CalcArea();
    };
    bool operator<(Figure* other) {
        return CalcArea() < other->CalcArea();
    };
    bool operator==(Figure* other) {
        return CalcArea() == other->CalcArea();
    };
    bool operator!=(Figure* other) {
        return CalcArea() != other->CalcArea();
    };
    void operator=(Figure* other) {
        SetX(other->GetX());
        SetY(other->GetY());
        SetZ(other->GetZ());
        SetColour(other->GetColour());
    };
    int GetX() {
        return this->_x;
    };
    int GetY() {
        return this->_y;
    };
    int GetZ() {
        return this->_z;
    };
    unsigned long GetColour() {
        return this->_color;
    };
    void SetX(int x) {
        this->_x = x;
    };
    void SetY(int y) {
        this->_y = y;
    };
    void SetZ(int z) {
        this->_z = z;
    };
    void SetColour(unsigned long color) {
        this->_color = color;
    };

    static string GetType() { return ""; }
    virtual double CalcArea() { return 0; };
    virtual string Show() { return ""; };
    virtual void Draw(wxDC&  dc) = 0;
    virtual bool IsClicked(int x, int y) { return false; };
    virtual void Save(ofstream& f)
    {
        f << GetX() << endl;
        f << GetY() << endl;
        f << GetZ() << endl;
        f << GetColour() << endl;
    };

    void Load(ifstream& f)
    {
        f >> _x;
        f >> _y;
        f >> _z;
        f >> _color;
    };
};

// Класс для кругов
class Circle: public Figure
{
private:
    float _r;
public:
    Circle(int x, int y, float r, unsigned long color): Figure(x,y,color) {
        _r = r;
    };
    Circle(ifstream& f): Figure(0,0,0) {
        Load(f);
    };
    Circle(Circle &copy): Circle(copy.GetX(), copy.GetY(), copy.GetRadius(), copy.GetColour()) {
        SetZ(copy.GetZ());
    };
    void operator=(Circle* other) {
        Figure::operator=((Figure*)other);
        SetRadius(other->GetRadius());
    };
    double CalcArea() {
        return M_PI * _r * _r;
    };
    string Show() {
        return fmt::format("Круг с центром в (x:{}, y:{}) и радиусом {}\nПлощадь: {:.2f}", GetX(), GetY(), GetRadius(), CalcArea());
    };
    static string GetType() {
        return "круг";
    };
    void Draw(wxDC&  dc) {
        brush->SetColour(wxColour(GetColour()));
        dc.SetBrush(*brush); 
        dc.SetPen( wxPen( wxColor(0,0,0), 1 ) ); 
        dc.DrawCircle( wxPoint(GetX(), GetY()), GetRadius());
    };
    void SetRadius(float r) {
        this->_r = r;
    };
    float GetRadius() {
        return _r;
    };
    bool IsClicked(int x, int y) {
        return sqrt(pow(x-GetX(), 2) + pow(y-GetY(), 2)) <= GetRadius();
    };
    void Save(ofstream& f) {
        f << GetType() << endl;
        Figure::Save(f);
        f << GetRadius() << endl;
    };
    void Load(ifstream& f) {
        Figure::Load(f);
        f >> _r;
    };
};

// Класс для прямоугольников
class Rectangle: public Figure
{
private:
    int _w, _h;
public:
    Rectangle(int x, int y, int w, int h, unsigned long color): Figure(x, y, color) {
        _w = w;
        _h = h;
    };
    Rectangle(ifstream& f): Figure(0,0,0) {
        Load(f);
    };
    Rectangle(Rectangle &copy): Rectangle(copy.GetX(), copy.GetY(), copy.GetWidth(), copy.GetHeight(), copy.GetColour()) {
        SetZ(copy.GetZ());
    };
    void operator=(Rectangle* other) {
        Figure::operator=((Figure*)other);
        SetWidth(other->GetWidth());
        SetHeight(other->GetHeight());
    };
    double CalcArea() {
        return _w * _h;
    };
    string Show() {
        return fmt::format("Прямоугольник с центром в (x:{}, y:{}), шириной {} и высотой {}\nПлощадь: {}", GetX(), GetY(), GetWidth(), GetHeight(), CalcArea());
    };
    static string GetType() {
        return "прямоугольник";
    };
    void Draw(wxDC&  dc) {
        brush->SetColour(wxColour(GetColour()));
        dc.SetBrush(*brush); 
        dc.SetPen( wxPen( wxColor(0,0,0), 1 ) ); 
        dc.DrawRectangle( GetX() - GetWidth() / 2, GetY() - GetHeight() / 2, GetWidth(), GetHeight());
    };
    int GetWidth() {
        return _w;
    };
    void SetWidth(int w) {
        _w = w;
    };
    int GetHeight() {
        return _h;
    };
    void SetHeight(int h) {
        _h = h;
    };
    bool IsClicked(int x, int y) {
        return abs(x-GetX()) <= GetWidth() / 2 && abs(y-GetY()) <= GetHeight() / 2;
    };
    void Save(ofstream& f) {
        f << GetType() << endl;
        Figure::Save(f);
        f << GetWidth() << endl;
        f << GetHeight() << endl;
    };
    void Load(ifstream& f) {
        Figure::Load(f);
        f >> _w;
        f >> _h;
    };
};

// Класс для треугольников
class Triangle: public Figure
{
private:
    int _a, _b, _c;
public:
    Triangle(int x, int y, int a, int b, int c, unsigned long color): Figure(x, y, color) {
        _a = a;
        _b = b;
        _c = c;
        checkSizes();
    };
    Triangle(ifstream& f): Figure(0,0,0) {
        Load(f);
        checkSizes();
    };
    Triangle(Triangle &copy): Triangle(copy.GetX(), copy.GetY(), copy.GetA(), copy.GetB(), copy.GetC(), copy.GetColour()) {
        SetZ(copy.GetZ());
    };
    void operator=(Triangle* other) {
        Figure::operator=((Figure*)other);
        SetA(other->GetA());
        SetB(other->GetB());
        SetC(other->GetC());
    };
    
    void checkSizes() {
        int minc = min(abs(_a-_b), abs(_b-_a));
        int maxc = _a+_b;
        if(_c < minc || _c > maxc) {
            throw WrongTriangleSizeException(_a,_b,_c);
        }
    };
    double CalcArea() {
        float s = (float)(_a+_b+_c) / 2;
        return sqrt(s*(s-_a)*(s-_b)*(s-_c));
    };
    string Show() {
        return fmt::format("Треугольник с левым углом в (x:{}, y:{}) и сторонами {}, {}, {}\nПлощадь: {:.2f}", GetX(), GetY(), GetA(), GetB(), GetC(), CalcArea());
    };
    static string GetType() {
        return "треугольник";
    };
    wxPoint *GetTrianglePoints() {
        wxPoint *points = new wxPoint[3];
        points[0] = wxPoint(GetX(),GetY());
        points[1] = wxPoint(GetX()+GetA(),GetY());

        float a1 = (pow(GetB(), 2) - pow(GetC(), 2) + pow(GetA(), 2)) / (2 * GetA()); 
        float h = sqrt(pow(GetB(), 2) - pow(a1, 2));
        wxPoint p = wxPoint(points[0].x + (a1*(points[1].x-points[0].x)/GetA()), points[0].y + (a1*(points[1].y-points[0].y)/GetA()));
        points[2] = wxPoint(p.x + (h*(points[1].y-points[0].y)/GetA()), p.y - (h*(points[1].x-points[0].x)/GetA()));
        return points;
    }
    void Draw(wxDC&  dc) {
        brush->SetColour(wxColour(GetColour()));
        dc.SetBrush(*brush); 
        dc.SetPen( wxPen( wxColor(0,0,0), 1 ) ); 
        
        wxPoint *points = GetTrianglePoints();
        dc.DrawPolygon(3, points);
        delete points;
    };
    int GetA() {
        return _a;
    };
    void SetA(int a) {
        _a = a;
        checkSizes();
    };
    int GetB() {
        return _b;
    };
    void SetB(int b) {
        _b = b;
        checkSizes();
    }
    int GetC() {
        return _c;
    };
    void SetC(int c) {
        _c = c;
        checkSizes();
    };
    bool IsClicked(int x, int y) {
        wxPoint *points = GetTrianglePoints();
        bool result = pointInTriangle(wxPoint(x,y), points[0], points[1], points[2]);
        delete points;
        return result;        
    };
    void Save(ofstream& f) {
        f << GetType() << endl;
        Figure::Save(f);
        f << GetA() << endl;
        f << GetB() << endl;
        f << GetC() << endl;
    };
    void Load(ifstream& f) {
        Figure::Load(f);
        f >> _a;
        f >> _b;
        f >> _c;
    };
};

const int MAX_SIZE = 255;
const string FILE_NAME = "figures.txt";
Figure* figures[MAX_SIZE];
int figuresCount = 0;
// Ссылка на перемещаемую фигуру
Figure* movingFigure = nullptr;
// Ссылка на фигуру, находящуюся под курсором
Figure* focusFigure = nullptr;
// Расстояние до центра после нажатия на фигуру
int ddX, ddY;
// Координаты мыши на канвасе
int mouseX = 0, mouseY = 0;

// Перемещение выбранной фигуры вперед по оси Z
void moveToFront(Figure** figures, int figuresCount, Figure* figure) {
    for(int i = 0; i < figuresCount; i++) {
        if(figures[i]->GetZ() < figure->GetZ()) {
            figures[i]->SetZ(figures[i]->GetZ()+1);
        }
    }
    figure->SetZ(0);
};

// Добавление фигуры в массив
void addFigure(Figure** figures, int &figuresCount, Figure* figure) {
    for(int i = 0; i < figuresCount; i++) {
        figures[i]->SetZ(figures[i]->GetZ()+1);
    }
    figure->SetZ(0);

    figures[figuresCount] = figure;
    figuresCount++;
};

void saveFigures(Figure** figures, int figuresCount) {
    ofstream f;
    try
    {
        f.exceptions(ofstream::failbit | ofstream::badbit);
        f.open(FILE_NAME);
        f.exceptions(std::ofstream::goodbit);

        for(int i = 0; i < figuresCount; i++) {
            figures[i]->Save(f);
        }
    }
    catch(ofstream::failure const &ex)
    {
        f.close();
        throw SaveException(ex.what());
    }
    f.close();
};

void loadFigures(Figure** figures, int &figuresCount) {
    ifstream f = ifstream(FILE_NAME);
    if(!f) {
        cout << "Загрузить не удалось" << endl;  
        return;
    }
    
    figuresCount = 0;
    try {
    while(!f.eof())
    {
        string type;
        f >> type;
        if(type == "") {
            break;
        }

        Figure *figure;
        if(type == Circle::GetType()) {
            Circle *circle = new Circle(f);
            figure = (Figure*)circle;
        }
        else if(type == Rectangle::GetType()) {
            Rectangle *rectangle = new Rectangle(f);
            figure = (Figure*)rectangle;
        }
        else if(type == Triangle::GetType()) {
            Triangle *triangle = new Triangle(f);
            figure = (Figure*)triangle;
        }
        else {
            throw WrongFigureTypeException(type);
        }
        if(figure) {
            figures[figuresCount] = figure;
            figuresCount++;
        }
    }
    } catch (...) {
        f.close();
        throw;
    }
};

// Добавляет круг случайного радиуса и по случайным координатам
// Но вычисляет координаты и радиус так, чтобы круг полностью находился в прямоугольнике со сторонами maxX, maxY 
void addRandomCircle(int maxX, int maxY) {
    int minRadius = 25;
    int x = minRadius + rand() % max(1, maxX - minRadius*2);
    int y = minRadius + rand() % max(1, maxY - minRadius*2);
    int radius = minRadius + rand() % (max(1, min({x, y, maxX - x, maxY - y}) - minRadius));
    Circle *circle = new Circle(x,y,radius,rand());
    cout << circle->Show() << endl;
    addFigure(figures, figuresCount, (Figure*)circle);
};

// Добавляет прямоугольник со случайной длиной и шириной и по случайным координатам
// Но вычисляет координаты и размер так, чтобы прямоугольник полностью находился в прямоугольнике со сторонами maxX, maxY 
void addRandomRectangle(int maxX, int maxY) {
    int minSize = 12;
    int x = minSize + rand() % max(1, maxX - minSize*2);
    int y = minSize + rand() % max(1, maxY - minSize*2);
    int width = minSize + rand() % (max(1, min({x, maxX - x}) - minSize));
    int height = minSize + rand() % (max(1, min({y, maxY - y}) - minSize));

    Rectangle *rectangle = new Rectangle(x,y,width*2,height*2,rand());
    cout << rectangle->Show() << endl;
    addFigure(figures, figuresCount, (Figure*)rectangle);
};

// Добавляет треугольник со случайными сторонами и по случайным координатам
// Сторону C вычисляет так, чтобы получившийся треугольник возможно было нарисовать
void addRandomTriangle(int maxX, int maxY) {
    int minSize = 25;
    int x = rand() % max(1, maxX - minSize);
    int y = rand() % max(1, maxY - minSize);
    int a = minSize + rand() % (max(1, min({x, maxX - x}) - minSize));
    int b = minSize + rand() % (y - minSize);
    int minc = abs(a-b);
    int maxc = a+b;
    int c = minc + (rand() % (maxc-minc));

    Triangle *triangle = new Triangle(x,y,a,b,c,rand());
    cout << triangle->Show() << endl;
    addFigure(figures, figuresCount, (Figure*)triangle);
};

// Класс для канваса
class BasicDrawPane : public wxPanel
{
public:
    BasicDrawPane(wxFrame* parent) : wxPanel(parent) {};
    
    void paintEvent(wxPaintEvent & evt);
    void paintNow();
    
    void render(wxDC& dc);
    
    void mouseMoved(wxMouseEvent& event);
    void mouseDown(wxMouseEvent& event);
    void mouseReleased(wxMouseEvent& event);
    void rightClick(wxMouseEvent& event);
    
    DECLARE_EVENT_TABLE()
};

// Основной класс приложения
class MyApp: public wxApp
{
    bool OnInit();
public:
    virtual bool OnExceptionInMainLoop()
    {
        string error;
        try {
            throw; // Rethrow the current exception.
        } catch (const LoadException& e) {
            error = e.getError();
        } catch (const SaveException& e) {
            error = e.getError();
        } catch (const WrongTriangleSizeException& e) {
            error = e.getError();
        } catch (const std::exception& e) {
            error = e.what();
        } catch ( ... ) {
            error = "unknown error.";
        }
 
        wxLogError("Unexpected exception has occurred: %s", error);
 
        return true;
    };

    BasicDrawPane* drawPane;
    void OnCircleBtnClick( wxCommandEvent& event );
    void OnRectangleBtnClick( wxCommandEvent& event );
    void OnTriangleBtnClick( wxCommandEvent& event );
    void OnSaveBtnClick( wxCommandEvent& event );
    void OnLoadBtnClick( wxCommandEvent& event );

    DECLARE_EVENT_TABLE()
};

enum
{
    BUTTON_Circle = wxID_HIGHEST + 1,
    BUTTON_Rectangle = wxID_HIGHEST + 2,
    BUTTON_Triangle = wxID_HIGHEST + 3,
    BUTTON_Save = wxID_HIGHEST + 4,
    BUTTON_Load = wxID_HIGHEST + 5,
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    // Основное окно
    wxFrame* frame = new wxFrame((wxFrame *)NULL, -1,  _T("Семестровая работа"), wxPoint(50,50), wxSize(800,600));
    // Карвас
    drawPane = new BasicDrawPane( (wxFrame*) frame );

    // Блок для отображения кнопок
    wxGridSizer *gs = new wxGridSizer(2, 3, 3, 3);
    gs->Add(new wxButton((wxFrame*) frame, BUTTON_Circle, _T("Круг")), 0, wxEXPAND);
    gs->Add(new wxButton((wxFrame*) frame, BUTTON_Rectangle, _T("Прямоугольник")), 0, wxEXPAND);
    gs->Add(new wxButton((wxFrame*) frame, BUTTON_Triangle, _T("Треугольник")), 0, wxEXPAND);
    gs->Add(new wxButton((wxFrame*) frame, BUTTON_Save, _T("Сохранить")), 0, wxEXPAND);
    gs->Add(new wxButton((wxFrame*) frame, BUTTON_Load, _T("Загрузить")), 0, wxEXPAND);

    // Блок - вертикальная колонка 
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(gs, 0, wxEXPAND);
    sizer->Add(drawPane, 1, wxEXPAND);
	
    // Назначаем основному окну блок со всеми элементами
    frame->SetSizer(sizer);
    // Обязательно вызываем SetAutoLayout, иначе размеры для блоков не пересчитаются
    frame->SetAutoLayout(true);
	
    frame->Show();
    return true;
};

BEGIN_EVENT_TABLE ( BasicDrawPane, wxPanel )
    EVT_MOTION ( BasicDrawPane::mouseMoved )
    EVT_LEFT_DOWN ( BasicDrawPane::mouseDown )
    EVT_LEFT_UP ( BasicDrawPane::mouseReleased )
    EVT_RIGHT_DOWN ( BasicDrawPane::rightClick )
    EVT_PAINT ( BasicDrawPane::paintEvent )
END_EVENT_TABLE()

BEGIN_EVENT_TABLE ( MyApp, wxApp )
    EVT_BUTTON ( BUTTON_Circle, MyApp::OnCircleBtnClick ) 
    EVT_BUTTON ( BUTTON_Rectangle, MyApp::OnRectangleBtnClick ) 
    EVT_BUTTON ( BUTTON_Triangle, MyApp::OnTriangleBtnClick ) 
    EVT_BUTTON ( BUTTON_Save, MyApp::OnSaveBtnClick ) 
    EVT_BUTTON ( BUTTON_Load, MyApp::OnLoadBtnClick ) 
END_EVENT_TABLE() 

void BasicDrawPane::mouseMoved(wxMouseEvent& event) {
    mouseX = event.GetX();
    mouseY = event.GetY();
    bool needToPaint = false;

    //Если мы сейчас перемещаем какую-нибудь фигуру, то меняем её координаты
    if(movingFigure) {
        movingFigure->SetX(mouseX - ddX);
        movingFigure->SetY(mouseY - ddY);
        needToPaint = true;
    }

    //Оптимизация - ищем фигуру для которой будем отображать подсказку
    Figure *f = nullptr;
    if(movingFigure) {
        f = movingFigure;
    } else {
        for(int z = 0; z < figuresCount; z++) {
            if(f) {
                break;
            }
            for(int i = 0; i < figuresCount; i++) {
                Figure *figure = figures[i];
                if(figure->GetZ() == z && figure->IsClicked(mouseX, mouseY)) {
                    f = figure;
                    break;
                }
            }
        }
    }

    // Перерисовываем канвас
    // Если нашли фигуру с подсказкой
    if(f) {
        focusFigure = f;
        needToPaint = true;
    } else if(focusFigure) { // Если фигуры для подсказки нет, но она раньше была
        focusFigure = nullptr;
        needToPaint = true;
    }
    if(needToPaint) {
        paintNow();
    }
};

// Событие нажатия кнопки мыши на канвас
void BasicDrawPane::mouseDown(wxMouseEvent& event) {
    if(focusFigure) {
        movingFigure = focusFigure;
        moveToFront(figures, figuresCount, movingFigure);
        paintNow();
        //Запоминаем где находилась мышь по отношению к центру перемещаемой фигуры
        ddX = event.GetX() - movingFigure->GetX();
        ddY = event.GetY() - movingFigure->GetY();
    }
};

void BasicDrawPane::mouseReleased(wxMouseEvent& event) {
    movingFigure = nullptr;
};

void BasicDrawPane::rightClick(wxMouseEvent& event) {
    if(focusFigure) {
        focusFigure->SetColour(rand());
        paintNow();
    }
};

void BasicDrawPane::paintEvent(wxPaintEvent & evt)
{
    wxPaintDC dc(this);
    render(dc);// Рисуем подсказку к фигуре в виде обведенного текста
};

void BasicDrawPane::paintNow()
{
    wxClientDC dc(this);
    render(dc);
};

void BasicDrawPane::render(wxDC&  dc)
{
    // Очистка канваса
    dc.Clear();
    // Рисуем фигруы по очереди, начиная с дальнего Z к ближнему
    for(int z = figuresCount-1; z >= 0; z--) {
        for(int i = 0; i < figuresCount; i++) {
            Figure *figure = figures[i];
            if(figure->GetZ() == z) {
                figure->Draw(dc);
                break;
            }   
        }
    }

    // Рисуем подсказку к фигуре в виде обведенного текста
    if(focusFigure) {
        int maxX = GetSize().GetWidth();
        int textWidth = 0;
        int textHeight = 0;
        auto ss = std::stringstream{focusFigure->Show()};
        for (std::string line; std::getline(ss, line, '\n');) {
            auto size = dc.GetTextExtent(line);
            textWidth = max(textWidth, size.GetWidth());
            textHeight += size.GetHeight();
        }
        
        int rightX = min(maxX, mouseX + textWidth);
        int topY = max(0, mouseY - textHeight);
        dc.SetTextForeground(wxColour(0,0,0));
        auto text = focusFigure->Show();
        dc.DrawText(text, rightX - textWidth-1, topY-1);
        dc.DrawText(text, rightX - textWidth+1, topY+1);
        dc.DrawText(text, rightX - textWidth-1, topY+1);
        dc.DrawText(text, rightX - textWidth+1, topY-1);
        dc.SetTextForeground(wxColour(255,255,255));
        dc.DrawText(text, rightX - textWidth, topY); 
    }
    
};

void MyApp::OnCircleBtnClick( wxCommandEvent& event ) {
    cout << "Добавляем круг" << endl;
    int maxX = drawPane->GetSize().GetWidth();
    int maxY = drawPane->GetSize().GetHeight();
    addRandomCircle(maxX, maxY);
    drawPane->paintNow();
};

void MyApp::OnRectangleBtnClick( wxCommandEvent& event ) {
    cout << "Добавляем прямоугольник" << endl;
    int maxX = drawPane->GetSize().GetWidth();
    int maxY = drawPane->GetSize().GetHeight();
    addRandomRectangle(maxX, maxY);
    drawPane->paintNow();
};

void MyApp::OnTriangleBtnClick( wxCommandEvent& event ) {
    cout << "Добавляем треугольник" << endl;
    int maxX = drawPane->GetSize().GetWidth();
    int maxY = drawPane->GetSize().GetHeight();
    addRandomTriangle(maxX, maxY);
    drawPane->paintNow();
};

void MyApp::OnSaveBtnClick( wxCommandEvent& event ) {
    cout << "Сохранение" << endl;
    saveFigures(figures, figuresCount);
};

void MyApp::OnLoadBtnClick( wxCommandEvent& event ) {
    cout << "Загрузка" << endl;
    try {
        loadFigures(figures, figuresCount);
    } catch (const WrongFigureTypeException &error) {
        throw LoadException(error.getError());
    }
    drawPane->paintNow();
};