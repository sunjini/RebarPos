//-----------------------------------------------------------------------------
//----- Utility.h : Declaration of utility functions
//-----------------------------------------------------------------------------
#pragma once

#include "acgi.h"
#include <string>

namespace Utility
{
	/// Creates a text style
	AcDbObjectId CreateTextStyle(const ACHAR* name, const ACHAR* filename, const double scale);

	/// Create an AcGiTextStyle from the db id of an AcDbTextStyleTableRecord
	Acad::ErrorStatus MakeGiTextStyle(AcGiTextStyle &ts, const AcDbObjectId  styleId);

	/// Gets the "0" layer
	const AcDbObjectId GetZeroLayer(void);

	/// Gets the "Defpoints" layer
	const AcDbObjectId GetDefpointsLayer(void);

	/// Replace all occurences of string
	const void ReplaceString(std::wstring& str, const std::wstring& oldStr, const std::wstring& newStr);

	/// Convert double to string
	const void DoubleToStr(const double val, const int digits, std::wstring& str);
	const void DoubleToStr(const double val, std::wstring& str);

	/// Convert string to double
	const double StrToDouble(const std::wstring& str);
	const double StrToDouble(const wchar_t* str);

	// Read DXF vals
	const Acad::ErrorStatus ReadDXFItem(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, resbuf* val);
	const Acad::ErrorStatus ReadDXFInt(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, short& val);
	const Acad::ErrorStatus ReadDXFUInt(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, unsigned short& val);
	const Acad::ErrorStatus ReadDXFLong(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, int& val);
	const Acad::ErrorStatus ReadDXFULong(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, unsigned int& val);
	const Acad::ErrorStatus ReadDXFLong(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, long& val);
	const Acad::ErrorStatus ReadDXFULong(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, unsigned long& val);
	const Acad::ErrorStatus ReadDXFReal(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, double& val);
	const Acad::ErrorStatus ReadDXFString(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, ACHAR*& val);
	const Acad::ErrorStatus ReadDXFPoint(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, AcGePoint2d& val);
	const Acad::ErrorStatus ReadDXFPoint(AcDbDxfFiler* pFiler, const short code, const ACHAR* name, AcGePoint3d& val);
}
