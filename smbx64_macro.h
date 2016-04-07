#ifndef SMBX64_MACRO_H
#define SMBX64_MACRO_H

/*! \file smbx64_macro.h
 *  \brief Contains helper macroses for making SMBX64 Standard format based parsers
 */

/*
  There are a special macroses used to organize
  SMBX64 specific file read functions.

  Skeleton of read file function:

  #include "file_formats.h"
  #include "file_strlist.h"
  #include "smbx64.h"
  #include "smbx64_macro.h"
  #include <iostream>

  void readFunc(QString rawData)
  {
     //Must be at begin of file
     SMBX64_File(rawData);

     //...

     std::cout << "File parsed!";
        return;
  badfile:
     std::cout << "File format is wrong!\n"<<
     << "line :" << str_count << ", file format version: "<<file_format
     << "Line data: "<< line;
  }

*/

/********************Read function begins***********************/
//for Header readers.
//Use it if you want read file partially
//(you must create QTextStream in(&fstream); !!!)
#define SMBX64_FileBegin() int file_format=0;   /*File format number*/\
                           PGESTRING line;      /*Current Line data*/

//for entire data readers.
//Use it if you want parse entire file data
#define SMBX64_File(raw) FileStringList in;\
                         in.addData( raw );\
                         SMBX64_FileBegin()

//Jump to next line
#define nextLine() line = in.readCVSLine();

#define parseLine(validate, target, converted) if( !validate ) \
                                                goto badfile;\
                                                 else target=converted;

//ValueTypes
#define strVar(target, line) parseLine( true/*SMBX64::qStr(line)*/, target, SMBX64::StrToStr(line))
#define UIntVar(target, line) parseLine( SMBX64::IsUInt(line), target, toInt(line))
#define SIntVar(target, line) parseLine( SMBX64::IsSInt(line), target, toInt(line))
#define wBoolVar(target, line) parseLine( SMBX64::IsCSVBool(line), target, SMBX64::wBoolR(line))
#ifdef PGE_FILES_QT
#define SFltVar(target, line) parseLine( SMBX64::IsFloat(line), target, line.replace(QChar(','), QChar('.')).toFloat());
#else
#define SFltVar(target, line) parseLine( SMBX64::IsFloat(line), target, ([line]() -> float { std::string newx=line;PGE_FileFormats_misc::replaceAll(newx, ",", "."); return toFloat(newx); })() )
#endif

#ifdef PGE_FILES_QT
#define strVarMultiLine(target, line) {\
bool first=true;\
while( (first && (line.size()==1)&&(line=="\""))||(!line.endsWith('\"')))\
{\
    first=false;\
    line.append('\n');\
    str_count++;line.append(in.readLine());\
    if(line.endsWith('\"'))\
        break;\
}\
strVar(target, line);\
}
#else
#define strVarMultiLine(target, line) {\
bool first=true;\
while( (first && (line.size()==1)&&(line=="\""))||(!PGE_FileFormats_misc::hasEnding(line, "\"")))\
{\
    first=false;\
    line.append("\n");\
    str_count++;line.append(in.readLine());\
    if(PGE_FileFormats_misc::hasEnding(line, "\""))\
        break;\
}\
strVar(target, line);\
}
#endif

//Version comparison
#define ge(v) file_format>=v
#define gt(v) file_format>v
#define le(v) file_format<=v
#define lt(v) file_format<v

#endif // SMBX64_MACRO_H

