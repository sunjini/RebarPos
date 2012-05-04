//-----------------------------------------------------------------------------
//----- PosShape.h : Declaration of PosShape
//-----------------------------------------------------------------------------

#pragma once

#include "..\NativeRebarPos\PosShape.h"

using namespace System;
using namespace Autodesk::AutoCAD::Geometry;
using namespace Autodesk::AutoCAD::DatabaseServices;

namespace OZOZ 
{
    namespace RebarPosWrapper 
    {
        [Autodesk::AutoCAD::Runtime::Wrapper("CPosShape")]
        public ref class PosShape :  public Autodesk::AutoCAD::DatabaseServices::DBObject
        {
		public:
			ref struct Shape
			{
			public:
				property Autodesk::AutoCAD::Colors::Color^ Color;

			protected:
				Shape(void)
				{
					Color = Autodesk::AutoCAD::Colors::Color::FromColorIndex(Autodesk::AutoCAD::Colors::ColorMethod::ByAci, 0);
				}
				Shape(Autodesk::AutoCAD::Colors::Color^ color)
				{
					Color = color;
				}
			};

			ref struct ShapeLine : Shape
			{
			public:
				property double X1;
				property double Y1;
				property double X2;
				property double Y2;

			public:
				ShapeLine(void) : Shape()
				{
					;
				}

				ShapeLine(Autodesk::AutoCAD::Colors::Color^ color, double x1, double y1, double x2, double y2) : Shape(color)
				{
					X1 = x1;
					Y1 = y1;
					X2 = x2;
					Y2 = y2;
				}

			};

			ref struct ShapeArc : Shape
			{
			public:
				property double X;
				property double Y;
				property double R;
				property double StartAngle;
				property double EndAngle;

			public:
				ShapeArc(void) : Shape()
				{
					;
				}

				ShapeArc(Autodesk::AutoCAD::Colors::Color^ color, double x, double y, double r, double startAngle, double endAngle) : Shape(color)
				{
					X = x;
					Y = y;
					R = r;
					StartAngle=startAngle;
					EndAngle=endAngle;
				}
			};

			ref struct ShapeText : Shape
			{
			public:
				property double X;
				property double Y;
				property double Height;
				property String^ Text;

			public:
				ShapeText(void) : Shape()
				{
					;
				}
				ShapeText(Autodesk::AutoCAD::Colors::Color^ color, double x, double y, double height, String^ text) : Shape(color)
				{
					X = x;
					Y = y;
					Height = height;
					Text = text;
				}
			};

		public:
			ref class ShapeCollection
			{
			private:
				PosShape^ m_Parent;

			internal:
				ShapeCollection(PosShape^ parent);

			public:
				void AddLine(double x1, double y1, double x2, double y2, Autodesk::AutoCAD::Colors::Color^ color);
				void AddArc(double x, double y, double r, double startAngle, double endAngle, Autodesk::AutoCAD::Colors::Color^ color);
				void AddText(double x, double y, double height, String^ text, Autodesk::AutoCAD::Colors::Color^ color);
				property int Count { int get(); }
				property Shape^ default[int] { Shape^ get(int index); void set(int index, Shape^ value); }
				void Remove(int index);
				void Clear();
			};

		protected:
			ShapeCollection^ m_Shapes;

        public:
            PosShape();

        internal:
            PosShape(System::IntPtr unmanagedPointer, bool autoDelete);
            inline CPosShape* GetImpObj()
            {
                return static_cast<CPosShape*>(UnmanagedObject.ToPointer());
            }

        public:
			property ShapeCollection^ Items { ShapeCollection^ get(); }

			property String^ Name           { String^ get(); void set(String^ value); }
			property int Fields             { int get(); void set(int value); }
			property String^ Formula        { String^ get(); void set(String^ value); }
			property String^ FormulaBending { String^ get(); void set(String^ value); }

			property int Priority { int get(); void set(int value); }

		public:
			property static String^ TableName        { String^ get(); }
        };
    }

}