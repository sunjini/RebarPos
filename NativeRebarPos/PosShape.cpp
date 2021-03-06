//-----------------------------------------------------------------------------
//----- PosShape.cpp : Implementation of CPosShape
//-----------------------------------------------------------------------------

#include "StdAfx.h"

#include "PosShape.h"
#include "Utility.h"

#include "FileWriteDescriptor.h"
#include "RowProviderInterface.h"
#include "FileSpecifier.h"
#include "DataModelAttributes.h"
#include "DataModel.h"
#include "RgbModel.h"
#include "Image.h"
#include "BmpFormatCodec.h"
#include "FormatCodecPropertySetIterator.h"

#include <fstream>
#include <algorithm>

std::map<std::wstring, CPosShape*> CPosShape::m_BuiltInPosShapes;
std::map<std::wstring, CPosShape*> CPosShape::m_InternalPosShapes;
std::map<std::wstring, CPosShape*> CPosShape::m_CustomPosShapes;

//*************************************************************************
// Code for the Class Body. 
//*************************************************************************

ACRX_CONS_DEFINE_MEMBERS(CPosShape, AcGiDrawable, 0);

//*************************************************************************
// Constructors and destructors 
//*************************************************************************
CPosShape::CPosShape () : m_Name(NULL), m_Fields(1), m_Formula(NULL), m_FormulaBending(NULL), 
	m_Priority(0), m_IsBuiltIn(Adesk::kFalse), m_IsUnknown(Adesk::kFalse), m_IsInternal(Adesk::kFalse),
	m_List(), m_A(NULL), m_B(NULL), m_C(NULL), m_D(NULL), m_E(NULL), m_F(NULL), m_Style(),
	m_GsNode(NULL)
{ }

CPosShape::~CPosShape () 
{ 
	acutDelString(m_Name);
	acutDelString(m_Formula);
	acutDelString(m_FormulaBending);

	acutDelString(m_A);
	acutDelString(m_B);
	acutDelString(m_C);
	acutDelString(m_D);
	acutDelString(m_E);
	acutDelString(m_F);

	ClearShapes();
}

//*************************************************************************
// Properties
//*************************************************************************
const ACHAR* CPosShape::Name(void) const
{
	return m_Name;
}
Acad::ErrorStatus CPosShape::setName(const ACHAR* newVal)
{
    acutDelString(m_Name);
    m_Name = NULL;
    if(newVal != NULL)
    {
        acutUpdString(newVal, m_Name);
    }
	return Acad::eOk;
}

const Adesk::Int32 CPosShape::Fields(void) const
{
	return m_Fields;
}

Acad::ErrorStatus CPosShape::setFields(const Adesk::Int32 newVal)
{
	m_Fields = newVal;
	return Acad::eOk;
}

const ACHAR* CPosShape::Formula(void) const
{
	return m_Formula;
}

Acad::ErrorStatus CPosShape::setFormula(const ACHAR* newVal)
{
    acutDelString(m_Formula);
    m_Formula = NULL;
    if(newVal != NULL)
    {
        acutUpdString(newVal, m_Formula);
    }

	return Acad::eOk;
}

const ACHAR* CPosShape::FormulaBending(void) const
{
	return m_FormulaBending;
}

Acad::ErrorStatus CPosShape::setFormulaBending(const ACHAR* newVal)
{
    acutDelString(m_FormulaBending);
    m_FormulaBending = NULL;
    if(newVal != NULL)
    {
        acutUpdString(newVal, m_FormulaBending);
    }

	return Acad::eOk;
}

const Adesk::Int32 CPosShape::Priority(void) const
{
	return m_Priority;
}

Acad::ErrorStatus CPosShape::setPriority(const Adesk::Int32 newVal)
{
    m_Priority = newVal;
	return Acad::eOk;
}

const Adesk::Boolean CPosShape::IsBuiltIn(void) const
{
	return m_IsBuiltIn;
}

Acad::ErrorStatus CPosShape::setIsBuiltIn(const Adesk::Boolean newVal)
{
	m_IsBuiltIn = newVal;
	return Acad::eOk;
}

const Adesk::Boolean CPosShape::IsUnknown(void) const
{
	return m_IsUnknown;
}

Acad::ErrorStatus CPosShape::setIsUnknown(const Adesk::Boolean newVal)
{
	m_IsUnknown = newVal;
	return Acad::eOk;
}

const Adesk::Boolean CPosShape::IsInternal(void) const
{
	return m_IsInternal;
}

Acad::ErrorStatus CPosShape::setIsInternal(const Adesk::Boolean newVal)
{
	m_IsInternal = newVal;
	return Acad::eOk;
}

HBITMAP CPosShape::ToBitmap(AcGsDevice* device, AcGsView* view, AcGsModel* model, const AcGsColor backColor, const int width, const int height)
{
	Atil::Image* image = ATILGetImage(device, view, model, backColor, width, height);
	HBITMAP hBmp = ATILConvertToBitmap(image);
	delete image;
	return hBmp;
}

HBITMAP CPosShape::ToBitmap(const AcGsColor backColor, const int width, const int height)
{
	AcGsManager* manager = acgsGetGsManager();

	AcGsKernelDescriptor descriptor;
	descriptor.addRequirement(AcGsKernelDescriptor::k3DDrawing);
	AcGsGraphicsKernel* kernel = manager->acquireGraphicsKernel(descriptor);

	AcGsDevice* device = kernel->createOffScreenDevice();
	device->onSize(width, height);
	device->setDeviceRenderer(AcGsDevice::kDefault);
	device->setBackgroundColor(backColor);

	AcGsView* view = kernel->createView();
	view->setVisualStyle(AcGiVisualStyle::k2DWireframe);

	AcGsModel* model = manager->createAutoCADModel(*kernel);

	device->add(view);

	HBITMAP hBmp = ToBitmap(device, view, model, backColor, width, height);

	device->eraseAll();

	manager->destroyAutoCADView(view);
	manager->destroyAutoCADModel(model);
	manager->destroyAutoCADDevice(device);

	return hBmp;
}

Atil::Image* CPosShape::ATILGetImage(AcGsDevice* device, AcGsView* view, AcGsModel* model, const AcGsColor backColor, const int width, const int height)
{
	device->setBackgroundColor(backColor);

	AcDbExtents extents;
	bounds(extents);
	AcGePoint3d pt1 = extents.minPoint();
	AcGePoint3d pt2 = extents.maxPoint();
	AcGePoint3d ptm((pt1.x + pt2.x) / 2.0, (pt1.y + pt2.y) / 2.0, 0.0);
	AcGePoint3d ptc(ptm.x, ptm.y, 1.0);
	double fw = abs(pt2.x - pt1.x);
	double fh = abs(pt2.y - pt1.y);
	view->setView(ptc, ptm, AcGeVector3d::kYAxis, fw, fh);

	view->add(this, model);
	view->update();
	
	Atil::RgbModel* datamodel = new Atil::RgbModel(Atil::RgbModelAttributes::k3Channels, Atil::DataModelAttributes::kRedGreenBlueAlpha);
	Atil::ImagePixel initialColor;
	initialColor.type = Atil::DataModelAttributes::kRgba;
	initialColor.value.rgba = Atil::RgbColor(backColor.m_red, backColor.m_green, backColor.m_blue);
	Atil::Image* image = new Atil::Image(Atil::Size(width, height), datamodel, initialColor);

	view->getSnapShot(image, AcGsDCPoint(0,0));

	view->eraseAll();

	return image;
}

HBITMAP CPosShape::ATILConvertToBitmap(Atil::Image* pImage)
{
	BITMAPINFOHEADER bih;
	ZeroMemory(&bih, sizeof(BITMAPINFOHEADER));
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = pImage->size().width;
	bih.biHeight = pImage->size().height;
	bih.biPlanes = 1;
	bih.biBitCount = 32;
	bih.biCompression = BI_RGB;
	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader = bih;

	// Fill in bitmap data
	DWORD* pPixels = (DWORD*)malloc(bih.biWidth * bih.biHeight * sizeof(DWORD));
	Atil::Size size = pImage->size();
	Atil::ImageContext* imgContext = pImage->createContext(Atil::ImageContext::kRead, size, Atil::Offset(0, 0));
	for (int x = 0; x < size.width; ++x)
	{
		for (int y = 0; y < size.height; ++y)
		{
			Atil::RgbColor col(imgContext->get32(x, y));
			pPixels[y * size.width + x] = RGB(col.rgba.blue, col.rgba.green, col.rgba.red);
		}
	}
	delete imgContext;

	char* ppvBits = NULL;
	HBITMAP hBitmap = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, (void**) &ppvBits, NULL, 0);
	SetDIBits(NULL, hBitmap, 0, bih.biHeight, pPixels, &bi, DIB_RGB_COLORS);

	return hBitmap;
}

//*************************************************************************
// Class Methods
//*************************************************************************
void CPosShape::AddShape(CShape* const shape)
{
	m_List.push_back(shape);
}

void CPosShape::InsertShape(const ShapeSize index, CShape* const shape)
{
	m_List.insert(m_List.begin() + index, shape);
}

const CShape* CPosShape::GetShape(const ShapeSize index) const
{
	return m_List.at(index);
}

void CPosShape::SetShape(const ShapeSize index, CShape* const shape)
{
	delete m_List[index];
	m_List[index] = shape;
}

void CPosShape::RemoveShape(const ShapeSize index)
{
	delete m_List[index];
	ShapeListIt it = m_List.begin();
	m_List.erase(it + index);
}

void CPosShape::ClearShapes()
{
	for(ShapeListIt it = m_List.begin(); it != m_List.end(); it++)
	{
		delete *it;
	}
	m_List.clear();
}

const ShapeSize CPosShape::GetShapeCount() const
{
	return m_List.size();
}

const ShapeListConstIt CPosShape::GetShapeIteratorBegin() const
{
	return m_List.begin();
}

const ShapeListConstIt CPosShape::GetShapeIteratorEnd() const
{
	return m_List.begin();
}

Acad::ErrorStatus CPosShape::setShapeTexts(const ACHAR* a, const ACHAR* b, const ACHAR* c, const ACHAR* d, const ACHAR* e, const ACHAR* f)
{
	clearShapeTexts();

	if ((a != NULL) && (a[0] != '\0')) acutUpdString(a, m_A);
	if ((b != NULL) && (b[0] != '\0')) acutUpdString(b, m_B);
	if ((c != NULL) && (c[0] != '\0')) acutUpdString(c, m_C);
	if ((d != NULL) && (d[0] != '\0')) acutUpdString(d, m_D);
	if ((e != NULL) && (e[0] != '\0')) acutUpdString(e, m_E);
	if ((f != NULL) && (f[0] != '\0')) acutUpdString(f, m_F);

	return Acad::eOk;
}
Acad::ErrorStatus CPosShape::clearShapeTexts(void)
{
	acutDelString(m_A);
	acutDelString(m_B);
	acutDelString(m_C);
	acutDelString(m_D);
	acutDelString(m_E);
	acutDelString(m_F);

	m_A = NULL;
	m_B = NULL;
	m_C = NULL;
	m_D = NULL;
	m_E = NULL;
	m_F = NULL;

	return Acad::eOk;
}

//*************************************************************************
// Common static dictionary methods
//*************************************************************************
void CPosShape::AddPosShape(CPosShape* shape)
{
	if(shape->IsInternal())
		m_InternalPosShapes[shape->Name()] = shape;
	else if(shape->IsBuiltIn())
		m_BuiltInPosShapes[shape->Name()] = shape;
	else
		m_CustomPosShapes[shape->Name()] = shape;
}

CPosShape* CPosShape::GetPosShape(const std::wstring& name)
{
	std::map<std::wstring, CPosShape*>::iterator it;
	if((it = m_InternalPosShapes.find(name)) != m_InternalPosShapes.end())
		return (*it).second;
	else if((it = m_BuiltInPosShapes.find(name)) != m_BuiltInPosShapes.end())
		return (*it).second;
	else if((it = m_CustomPosShapes.find(name)) != m_CustomPosShapes.end())
		return (*it).second;

	CPosShape* shape = CPosShape::GetUnknownPosShape();
	shape->setName(name.c_str());
	return shape;
}

CPosShape* CPosShape::GetUnknownPosShape(void)
{
	CPosShape* shape = new CPosShape();

	shape->setName(_T("HATA"));
	shape->setFields(1);
	shape->setFormula(_T("A"));
	shape->setFormulaBending(_T("A"));
	shape->setIsBuiltIn(Adesk::kTrue);
	shape->setIsUnknown(Adesk::kTrue);

	return shape;
}

bool CPosShape::HasPosShape(const std::wstring& name)
{
	if(m_InternalPosShapes.find(name) != m_InternalPosShapes.end())
		return true;
	else if(m_BuiltInPosShapes.find(name) != m_BuiltInPosShapes.end())
		return true;
	else if(m_CustomPosShapes.find(name) != m_CustomPosShapes.end())
		return true;

	return false;
}

int CPosShape::GetPosShapeCount(const bool builtin, const bool isinternal, const bool custom)
{
	int count = 0;

	if(isinternal) count += (int)m_InternalPosShapes.size();
	if(builtin) count += (int)m_BuiltInPosShapes.size();
	if(custom) count += (int)m_CustomPosShapes.size();

	return count;
}

void CPosShape::ClearPosShapes(const bool builtin, const bool isinternal, const bool custom)
{
	if(isinternal && !m_InternalPosShapes.empty())
	{
		for(std::map<std::wstring, CPosShape*>::iterator it = m_InternalPosShapes.begin(); it != m_InternalPosShapes.end(); it++)
		{
			delete (*it).second;
		}
		m_InternalPosShapes.clear();
	}
	if(builtin && !m_BuiltInPosShapes.empty())
	{
		for(std::map<std::wstring, CPosShape*>::iterator it = m_BuiltInPosShapes.begin(); it != m_BuiltInPosShapes.end(); it++)
		{
			delete (*it).second;
		}
		m_BuiltInPosShapes.clear();
	}
	if(custom && !m_CustomPosShapes.empty())
	{
		for(std::map<std::wstring, CPosShape*>::iterator it = m_CustomPosShapes.begin(); it != m_CustomPosShapes.end(); it++)
		{
			delete (*it).second;
		}
		m_CustomPosShapes.clear();
	}
}

std::vector<std::wstring> CPosShape::GetAllShapes(const bool builtin, const bool isinternal, const bool custom)
{
	std::vector<std::wstring> names;
	if(isinternal && !m_InternalPosShapes.empty())
	{
		for(std::map<std::wstring, CPosShape*>::iterator it = m_InternalPosShapes.begin(); it != m_InternalPosShapes.end(); it++)
		{
			names.push_back(std::wstring(it->first));
		}
	}
	if(builtin && !m_BuiltInPosShapes.empty())
	{
		for(std::map<std::wstring, CPosShape*>::iterator it = m_BuiltInPosShapes.begin(); it != m_BuiltInPosShapes.end(); it++)
		{
			names.push_back(std::wstring(it->first));
		}
	}
	if(custom && !m_CustomPosShapes.empty())
	{
		for(std::map<std::wstring, CPosShape*>::iterator it = m_CustomPosShapes.begin(); it != m_CustomPosShapes.end(); it++)
		{
			names.push_back(std::wstring(it->first));
		}
	}

	std::sort(names.begin(), names.end(), CPosShape::SortShapeNames);
	return names;
}

bool CPosShape::SortShapeNames(const std::wstring& p1, const std::wstring& p2)
{
	if (p1 == p2) return true;

	if (p1 == L"GENEL")
		return false;
	else if (p2 == L"GENEL")
		return true;
	else
	{
		if(Utility::IsNumeric(p1) && Utility::IsNumeric(p2))
		{
			int n1 = Utility::StrToInt(p1);
			int n2 = Utility::StrToInt(p2);
			return n1 < n2;
		}
		else
		{
			return p1.compare(p2) < 0;
		}
	}
}

void CPosShape::ReadPosShapesFromResource(const HINSTANCE hInstance, const int resid, const bool isinternal)
{
	std::wstring source = Utility::StringFromResource(hInstance, L"SHAPELIST", resid);
	ReadPosShapesFromString(source, true, isinternal);
}

void CPosShape::ReadPosShapesFromFile(const std::wstring& filename)
{
	std::wstring source = Utility::StringFromFile(filename);
	ReadPosShapesFromString(source, false, false);
}

void CPosShape::ReadPosShapesFromString(const std::wstring& source, const bool builtin, const bool isinternal)
{
	std::wistringstream stream(source);
	std::wstring   line;
	
	while(std::getline(stream, line))
	{
		while(!stream.eof() && line.compare(0, 5, L"BEGIN") != 0)
			std::getline(stream, line);

		if(stream.eof())
			break;

		std::wstringstream linestream;
		std::wstring fieldname;
		std::wstring name;
		int fields;
		std::wstring formula;
		std::wstring formulabending;
		int priority;
		int count;

		// Name
		std::getline(stream, line);
		linestream.clear(); linestream.str(std::wstring());
		linestream << line;
		std::getline(linestream, fieldname, L'\t');
		linestream >> name;
		// Fields
		std::getline(stream, line);
		linestream.clear(); linestream.str(std::wstring());
		linestream << line;
		std::getline(linestream, fieldname, L'\t');
		linestream >> fields;
		// Formula
		std::getline(stream, line);
		linestream.clear(); linestream.str(std::wstring());
		linestream << line;
		std::getline(linestream, fieldname, L'\t');
		linestream >> formula;
		// FormulaBending
		std::getline(stream, line);
		linestream.clear(); linestream.str(std::wstring());
		linestream << line;
		std::getline(linestream, fieldname, L'\t');
		linestream >> formulabending;
		// Priority
		std::getline(stream, line);
		linestream.clear(); linestream.str(std::wstring());
		linestream << line;
		std::getline(linestream, fieldname, L'\t');
		linestream >> priority;
		// Count
		std::getline(stream, line);
		linestream.clear(); linestream.str(std::wstring());
		linestream << line;
		std::getline(linestream, fieldname, L'\t');
		linestream >> count;

		// Create the shape
		CPosShape *shape = new CPosShape();
		shape->setName(name.c_str());
		shape->setFields(fields);
		shape->setFormula(formula.c_str());
		shape->setFormulaBending(formulabending.c_str());
		shape->setIsBuiltIn(builtin ? Adesk::kTrue : Adesk::kFalse);
		shape->setIsInternal(isinternal ? Adesk::kTrue : Adesk::kFalse);
		
		// Read shapes
		for(int i = 0; i < count; i++)
		{
			std::getline(stream, line);
			linestream.clear(); linestream.str(std::wstring());
			linestream << line;
			std::getline(linestream, fieldname, L'\t');

			if(fieldname.compare(L"LINE") == 0)
			{
				double x1, y1, x2, y2;
				unsigned short color;
				std::wstring visible;
				linestream >> x1 >> y1 >> x2 >> y2 >> color >> visible;

				CShapeLine *cline = new CShapeLine(color, x1, y1, x2, y2, (visible.compare(L"V") == 0 ? Adesk::kTrue : Adesk::kFalse));
				shape->AddShape(cline);
			}
			else if(fieldname.compare(L"ARC") == 0)
			{
				double x, y, r, a1, a2;
				unsigned short color;
				std::wstring visible;
				linestream >> x >> y >> r >> a1 >> a2 >> color >> visible;

				CShapeArc *arc = new CShapeArc(color, x, y, r, a1, a2, (visible.compare(L"V") == 0 ? Adesk::kTrue : Adesk::kFalse));
				shape->AddShape(arc);
			}
			else if(fieldname.compare(L"CIRCLE") == 0)
			{
				double x, y, r;
				unsigned short color;
				std::wstring visible;
				linestream >> x >> y >> r >> color >> visible;

				CShapeCircle *circle = new CShapeCircle(color, x, y, r, (visible.compare(L"V") == 0 ? Adesk::kTrue : Adesk::kFalse));
				shape->AddShape(circle);
			}
			else if(fieldname.compare(L"ELLIPSE") == 0)
			{
				double x, y, width, height;
				unsigned short color;
				std::wstring visible;
				linestream >> x >> y >> width >> height >> color >> visible;

				CShapeEllipse *ellipse = new CShapeEllipse(color, x, y, width, height, (visible.compare(L"V") == 0 ? Adesk::kTrue : Adesk::kFalse));
				shape->AddShape(ellipse);
			}
			else if(fieldname.compare(L"TEXT") == 0)
			{
				double x, y, h, w;
				std::wstring font;
				std::wstring str;
				int ha, va;
				unsigned short color;
				std::wstring visible;
				linestream >> x >> y >> h >> w >> str >> font >> ha >> va >> color >> visible;

				CShapeText *text = new CShapeText(color, x, y, h, w, str.c_str(), font.c_str(), static_cast<AcDb::TextHorzMode>(ha), static_cast<AcDb::TextVertMode>(va), (visible.compare(L"V") == 0 ? Adesk::kTrue : Adesk::kFalse));
				shape->AddShape(text);
			}
		}


		// Add the shape to the dictionary
		shape->setIsInternal(isinternal ? Adesk::kTrue : Adesk::kFalse);
		shape->setIsBuiltIn(builtin ? Adesk::kTrue : Adesk::kFalse);
		AddPosShape(shape);
	}
}


void CPosShape::SavePosShapesToFile(const std::wstring& filename)
{
	std::wstringstream ofs;
	ofs.clear(); ofs.str(std::wstring());

	if(!m_CustomPosShapes.empty())
	{
		for (std::map<std::wstring, CPosShape*>::iterator it = m_CustomPosShapes.begin(); it != m_CustomPosShapes.end(); it++)
		{
			CPosShape* posShape = (*it).second;

			ofs << L"BEGIN" << std::endl;

			ofs << L"Name" << L'\t' << posShape->Name() << std::endl;
			ofs << L"Fields" << L'\t' << posShape->Fields() << std::endl;
			ofs << L"Formula" << L'\t' << posShape->Formula() << std::endl;
			ofs << L"FormulaBending" << L'\t' << posShape->FormulaBending() << std::endl;
			ofs << L"Priority" << L'\t' << posShape->Priority() << std::endl;
			ofs << L"Count" << L'\t' << posShape->GetShapeCount() << std::endl;

			// Write shapes
			for (ShapeSize i = 0; i < posShape->GetShapeCount(); i++)
			{
				const CShape* shape = posShape->GetShape(i);
				if (shape->type == CShape::Line)
				{
					const CShapeLine* line = dynamic_cast<const CShapeLine*>(shape);
					ofs << L"LINE" << L'\t' <<
						line->x1 << L'\t' << line->y1 << L'\t' << line->x2 << L'\t' << line->y2 << L'\t' <<
						line->color << L'\t' << (line->visible == Adesk::kTrue ? L'V' : L'I') << std::endl;
				}
				else if (shape->type == CShape::Arc)
				{
					const CShapeArc* arc = dynamic_cast<const CShapeArc*>(shape);
					ofs << L"ARC" << L'\t' <<
						arc->x << L'\t' << arc->y << L'\t' << arc->r << L'\t' <<
						arc->startAngle << L'\t' << arc->endAngle << L'\t' <<
						arc->color << L'\t' << (arc->visible == Adesk::kTrue ? L'V' : L'I') << std::endl;
				}
				else if (shape->type == CShape::Circle)
				{
					const CShapeCircle* circle = dynamic_cast<const CShapeCircle*>(shape);
					ofs << L"CIRCLE" << L'\t' <<
						circle->x << L'\t' << circle->y << L'\t' << circle->r << L'\t' <<
						circle->color << L'\t' << (circle->visible == Adesk::kTrue ? L'V' : L'I') << std::endl;
				}
				else if (shape->type == CShape::Ellipse)
				{
					const CShapeEllipse* ellipse = dynamic_cast<const CShapeEllipse*>(shape);
					ofs << L"ELLIPSE" << L'\t' <<
						ellipse->x << L'\t' << ellipse->y << L'\t' << ellipse->width << L'\t' << ellipse->height << L'\t' <<
						ellipse->color << L'\t' << (ellipse->visible == Adesk::kTrue ? L'V' : L'I') << std::endl;
				}
				else if (shape->type == CShape::Text)
				{
					const CShapeText* text = dynamic_cast<const CShapeText*>(shape);
					ofs << L"TEXT" << L'\t' <<
						text->x << L'\t' << text->y << L'\t' << text->height << L'\t' << text->width << L'\t' <<
						text->text << L'\t' << text->font << L'\t' <<
						text->horizontalAlignment << L'\t' << text->verticalAlignment << L'\t' <<
						text->color << L'\t' << (text->visible == Adesk::kTrue ? L'V' : L'I') << std::endl;
				}
			}

			ofs << L"END" << std::endl;
		}
	}

	Utility::StringToFile(filename, ofs.str());
}

//*************************************************************************
// RXObject implementation
//*************************************************************************
Acad::ErrorStatus CPosShape::copyFrom(const AcRxObject* other)
{
	if(!other->isKindOf(CPosShape::desc())) return Acad::eNotThatKindOfClass;
	CPosShape* org = CPosShape::cast(other);

	setName(org->Name());
	setFields(org->Fields());
	setFormula(org->Formula());
	setFormulaBending(org->FormulaBending());
	setPriority(org->Priority());

	setIsUnknown(org->IsUnknown());
	setIsBuiltIn(org->IsBuiltIn());
	setIsInternal(org->IsInternal());

	ClearShapes();
	for(ShapeSize i = 0; i < org->GetShapeCount(); i++)
	{
		AddShape(org->GetShape(i)->clone());
	}

	return Acad::eOk;
}

//*************************************************************************
// Drawable implementation
//*************************************************************************
Adesk::Boolean CPosShape::isPersistent(void) const
{
	return Adesk::kFalse;
}

AcDbObjectId CPosShape::id(void) const
{
	return AcDbObjectId::kNull;
}

#ifdef REBARPOS2010
void CPosShape::setGsNode(AcGsNode* gsnode)
{
	m_GsNode = gsnode;
}

AcGsNode* CPosShape::gsNode(void) const
{
	return m_GsNode;
}
#endif

bool CPosShape::bounds(AcDbExtents& ext) const
{
	if(m_List.empty())
	{
		return false;
	}
	else
	{
		for(ShapeListConstIt it = m_List.begin(); it != m_List.end(); it++)
		{
			CShape* shape = *it;
			switch(shape->type)
			{
			case CShape::Line:
				{
					CShapeLine* line = dynamic_cast<CShapeLine*>(shape);
					ext.addPoint(AcGePoint3d(line->x1, line->y1, 0));
					ext.addPoint(AcGePoint3d(line->x2, line->y2, 0));
				}
				break;
			case CShape::Arc:
				{
					CShapeArc* arc = dynamic_cast<CShapeArc*>(shape);
					double da = (arc->endAngle - arc->startAngle) / 10.0;
					int i = 0;
					double a = arc->startAngle;
					for(i = 0; i < 10; i++)
					{
						double x = arc->x + cos(a) * arc->r;
						double y = arc->y + sin(a) * arc->r;
						ext.addPoint(AcGePoint3d(x, y, 0));
						a += da;
					}
				}
				break;
			case CShape::Circle:
				{
					CShapeCircle* circle = dynamic_cast<CShapeCircle*>(shape);
					ext.addPoint(AcGePoint3d(circle->x - circle->r, circle->y - circle->r, 0));
					ext.addPoint(AcGePoint3d(circle->x - circle->r, circle->y + circle->r, 0));
					ext.addPoint(AcGePoint3d(circle->x + circle->r, circle->y + circle->r, 0));
					ext.addPoint(AcGePoint3d(circle->x + circle->r, circle->y - circle->r, 0));
				}
				break;
			case CShape::Ellipse:
				{
					CShapeEllipse* ellipse = dynamic_cast<CShapeEllipse*>(shape);
					ext.addPoint(AcGePoint3d(ellipse->x - ellipse->width / 2.0, ellipse->y - ellipse->height / 2.0, 0));
					ext.addPoint(AcGePoint3d(ellipse->x - ellipse->width / 2.0, ellipse->y + ellipse->height / 2.0, 0));
					ext.addPoint(AcGePoint3d(ellipse->x + ellipse->width / 2.0, ellipse->y + ellipse->height / 2.0, 0));
					ext.addPoint(AcGePoint3d(ellipse->x + ellipse->width / 2.0, ellipse->y - ellipse->height / 2.0, 0));
				}
				break;
			case CShape::Text:
				{
					CShapeText* text = dynamic_cast<CShapeText*>(shape);

					std::wstring txt(text->text);
					if(m_A != NULL) Utility::ReplaceString(txt, L"A", m_A);
					if(m_B != NULL) Utility::ReplaceString(txt, L"B", m_B);
					if(m_C != NULL) Utility::ReplaceString(txt, L"C", m_C);
					if(m_D != NULL) Utility::ReplaceString(txt, L"D", m_D);
					if(m_E != NULL) Utility::ReplaceString(txt, L"E", m_E);
					if(m_F != NULL) Utility::ReplaceString(txt, L"F", m_F);

					AcGiTextStyle style(text->font.c_str(), NULL, text->height, text->width, 0, 0, Adesk::kFalse, Adesk::kFalse, Adesk::kFalse, Adesk::kFalse, Adesk::kFalse);
					style.loadStyleRec();
					AcGePoint2d size = Utility::MeasureText(txt, style);

					double xoff = 0.0; 
					if(text->horizontalAlignment == AcDb::kTextLeft)
						xoff = 0.0;
					else if(text->horizontalAlignment == AcDb::kTextRight)
						xoff = -size.x;
					else // horizontal center
						xoff = -size.x / 2.0;

					double yoff = 0.0;
					if(text->verticalAlignment == AcDb::kTextTop) 
						yoff = -size.y;
					else if(text->verticalAlignment == AcDb::kTextBase || text->verticalAlignment == AcDb::kTextBottom)
						yoff = 0.0;
					else // vertical middle
						yoff = -size.y / 2.0;
					
					ext.addPoint(AcGePoint3d(text->x + xoff, text->y + yoff, 0));
					ext.addPoint(AcGePoint3d(text->x + xoff + size.x, text->y + yoff + size.y, 0));
				}
				break;
			}
		}

		return true;
	}
}

Adesk::UInt32 CPosShape::subSetAttributes(AcGiDrawableTraits* traits)
{
	return AcGiDrawable::kDrawableNone;
}

Adesk::Boolean CPosShape::subWorldDraw(AcGiWorldDraw* worldDraw)
{
    if(worldDraw->regenAbort())
	{
        return Adesk::kTrue;
    }

	AcDbExtents ext;
	if(!bounds(ext))
	{
		return Adesk::kTrue;
	}

	for(ShapeListConstIt it = m_List.begin(); it != m_List.end(); it++)
	{
		CShape* shape = *it;
		if(shape->visible == Adesk::kFalse) continue;

		switch(shape->type)
		{
		case CShape::Line:
			{
				CShapeLine* line = dynamic_cast<CShapeLine*>(shape);
				Utility::DrawLine(worldDraw, AcGePoint3d(line->x1, line->y1, 0), AcGePoint3d(line->x2, line->y2, 0), line->color);
			}
			break;
		case CShape::Arc:
			{
				CShapeArc* arc = dynamic_cast<CShapeArc*>(shape);
				Utility::DrawArc(worldDraw, AcGePoint3d(arc->x, arc->y, 0), arc->r, arc->startAngle, arc->endAngle, arc->color);
			}
			break;
		case CShape::Circle:
			{
				CShapeCircle* circle = dynamic_cast<CShapeCircle*>(shape);
				Utility::DrawCircle(worldDraw, AcGePoint3d(circle->x, circle->y, 0), circle->r, circle->color);
			}
			break;
		case CShape::Ellipse:
			{
				CShapeEllipse* ellipse = dynamic_cast<CShapeEllipse*>(shape);
				Utility::DrawEllipse(worldDraw, AcGePoint3d(ellipse->x, ellipse->y, 0), ellipse->width, ellipse->height, ellipse->color);
			}
			break;
		case CShape::Text:
			{
				CShapeText* text = dynamic_cast<CShapeText*>(shape);

				std::wstring txt(text->text);
				if(m_A != NULL) Utility::ReplaceString(txt, L"A", m_A);
				if(m_B != NULL) Utility::ReplaceString(txt, L"B", m_B);
				if(m_C != NULL) Utility::ReplaceString(txt, L"C", m_C);
				if(m_D != NULL) Utility::ReplaceString(txt, L"D", m_D);
				if(m_E != NULL) Utility::ReplaceString(txt, L"E", m_E);
				if(m_F != NULL) Utility::ReplaceString(txt, L"F", m_F);

				if(text->height != m_Style.textSize() || text->width != m_Style.xScale() || text->font.compare(m_Style.fileName()) != 0)
				{
					m_Style.setFileName(text->font.c_str());
					m_Style.setTextSize(text->height);
					m_Style.setXScale(text->width);
					m_Style.loadStyleRec();
				}
				Utility::DrawText(worldDraw, AcGePoint3d(text->x, text->y, 0), txt, m_Style, text->horizontalAlignment, text->verticalAlignment, text->color);
			}
			break;
		}
	}

	// Do not call viewportDraw()
    return Adesk::kTrue; 
}

void CPosShape::subViewportDraw(AcGiViewportDraw* vd)
{
	;
}

Adesk::UInt32 CPosShape::subViewportDrawLogicalFlags(AcGiViewportDraw* vd)
{
	return 0;
}