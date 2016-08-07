/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014-2016 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "file_formats.h"
#include "file_strlist.h"
#include "pge_x.h"

#ifdef PGE_EDITOR
#include <script/commands/memorycommand.h>
#endif

//*********************************************************
//****************READ FILE FORMAT*************************
//*********************************************************

bool FileFormats::ReadNonSMBX64MetaDataF(PGESTRING filePath, MetaData &FileData)
{
    errorString.clear();
    PGE_FileFormats_misc::TextFileInput file;
    if(!file.open(filePath, true))
    {
        errorString="Failed to open file for read";
        FileData.meta.ERROR_info = errorString;
        FileData.meta.ERROR_linedata = "";
        FileData.meta.ERROR_linenum = -1;
        FileData.meta.ReadFileValid = false;
        return false;
    }
    return ReadNonSMBX64MetaDataFile(file, FileData);
}

bool FileFormats::ReadNonSMBX64MetaDataRaw(PGESTRING &rawdata, PGESTRING filePath, MetaData &FileData)
{
    errorString.clear();
    PGE_FileFormats_misc::RawTextInput file;
    if(!file.open(&rawdata, filePath))
    {
        errorString="Failed to open raw string for read";
        FileData.meta.ERROR_info = errorString;
        FileData.meta.ERROR_linedata = "";
        FileData.meta.ERROR_linenum = -1;
        FileData.meta.ReadFileValid = false;
        return false;
    }
    return ReadNonSMBX64MetaDataFile(file, FileData);
}

bool FileFormats::ReadNonSMBX64MetaDataFile(PGE_FileFormats_misc::TextInput &in, MetaData &FileData)
{
    PGESTRING errorString;
    int str_count=0;        //Line Counter
    PGESTRING line;           //Current Line data

    #ifdef PGE_EDITOR
    FileData.script.reset(new ScriptHolder());
    #endif
    ///////////////////////////////////////Begin file///////////////////////////////////////
    PGEFile pgeX_Data(in.readAll());
    if( !pgeX_Data.buildTreeFromRaw() )
    {
        errorString = pgeX_Data.lastError();
        goto badfile;
    }

    for(int section=0; section<(signed)pgeX_Data.dataTree.size(); section++) //look sections
    {
        PGEFile::PGEX_Entry &f_section = pgeX_Data.dataTree[section];
        if(f_section.name=="META_BOOKMARKS")
        {
            if(f_section.type!=PGEFile::PGEX_Struct)
            {
                errorString=PGESTRING("Wrong section data syntax:\nSection [")+f_section.name+"%1]";
                goto badfile;
            }

            for(int sdata=0;sdata<(signed)f_section.data.size();sdata++)
            {
                if(f_section.data[sdata].type!=PGEFile::PGEX_Struct)
                {
                    errorString=PGESTRING("Wrong data item syntax:\nSection [")+
                            f_section.name+"]\nData line "+
                            fromNum(sdata)+")";
                    goto badfile;
                }

                PGEFile::PGEX_Item x = f_section.data[sdata];

                Bookmark meta_bookmark;
                meta_bookmark.bookmarkName = "";
                meta_bookmark.x = 0;
                meta_bookmark.y = 0;

                for(int sval=0;sval<(signed)x.values.size();sval++) //Look markers and values
                {
                    PGEFile::PGEX_Val v = x.values[sval];
                    errorString=PGESTRING("Wrong value syntax\nSection [")+
                            f_section.name+"]\nData line "+
                            fromNum(sdata)+"\nMarker "+
                            v.marker+"\nValue "+
                            v.value;

                      if(v.marker=="BM") //Bookmark name
                      {
                          if(PGEFile::IsQoutedString(v.value))
                              meta_bookmark.bookmarkName = PGEFile::X2STRING(v.value);
                          else
                              goto badfile;
                      }
                      else
                      if(v.marker=="X") // Position X
                      {
                          if(PGEFile::IsIntS(v.value))
                              meta_bookmark.x = toInt(v.value);
                          else
                              goto badfile;
                      }
                      else
                      if(v.marker=="Y") //Position Y
                      {
                          if(PGEFile::IsIntS(v.value))
                              meta_bookmark.y = toInt(v.value);
                          else
                              goto badfile;
                      }
                }
                FileData.bookmarks.push_back(meta_bookmark);
            }
        }
        #ifdef PGE_EDITOR
        else
        if(f_section.name=="META_SCRIPT_EVENTS")
        {
            if(f_section.type!=PGEFile::PGEX_Struct)
            {
                errorString=QString("Wrong section data syntax:\nSection [%1]").
                            arg(f_section.name);
                goto badfile;
            }

            if(f_section.subTree.size()>0)
            {
                if(!FileData.script)
                    FileData.script.reset(new ScriptHolder());
            }

            //Read subtree
            for(int subtree=0;subtree<f_section.subTree.size();subtree++)
            {
                if(f_section.subTree[subtree].name=="EVENT")
                {
                    if(f_section.type!=PGEFile::PGEX_Struct)
                    {
                        errorString=QString("Wrong section data syntax:\nSection [%1]\nSubtree [%2]").
                                    arg(f_section.name).
                                    arg(f_section.subTree[subtree].name);
                        goto badfile;
                    }

                    EventCommand * event = new EventCommand(EventCommand::EVENTTYPE_LOAD);

                    for(int sdata=0;sdata<f_section.subTree[subtree].data.size();sdata++)
                    {
                        if(f_section.subTree[subtree].data[sdata].type!=PGEFile::PGEX_Struct)
                        {
                            errorString=QString("Wrong data item syntax:\nSubtree [%1]\nData line %2")
                                    .arg(f_section.subTree[subtree].name)
                                    .arg(sdata);
                            goto badfile;
                        }

                        PGEFile::PGEX_Item x = f_section.subTree[subtree].data[sdata];

                        //Get values
                        for(int sval=0;sval<x.values.size();sval++) //Look markers and values
                        {
                            PGEFile::PGEX_Val v = x.values[sval];
                            errorString=QString("Wrong value syntax\nSection [%1]\nData line %2\nMarker %3\nValue %4")
                                    .arg(f_section.name)
                                    .arg(sdata)
                                    .arg(v.marker)
                                    .arg(v.value);

                            if(v.marker=="TL") //Marker
                            {
                                if(PGEFile::IsQoutedString(v.value))
                                    event->setMarker(PGEFile::X2STRING(v.value));
                                else
                                    goto badfile;
                            }
                            else
                            if(v.marker=="ET") //Event type
                            {
                                if(PGEFile::IsIntS(v.value))
                                    event->setEventType( (EventCommand::EventType)v.value.toInt() );
                                else
                                    goto badfile;
                            }
                        }
                    }//Event header

                    //Basic commands subtree
                    if(event->eventType()==EventCommand::EVENTTYPE_LOAD)
                    {
                        for(int subtree2=0;subtree2<f_section.subTree[subtree].subTree.size();subtree2++)
                        {
                            if(f_section.subTree[subtree2].subTree[subtree2].name=="BASIC_COMMANDS")
                            {
                                if(f_section.type!=PGEFile::PGEX_Struct)
                                {
                                    errorString=QString("Wrong section data syntax:\nSection [%1]\nSubtree [%2]\nSubtree [%2]").
                                                arg(f_section.name).
                                                arg(f_section.subTree[subtree].name).
                                                arg(f_section.subTree[subtree].subTree[subtree2].name);
                                    goto badfile;
                                }

                                for(int sdata=0;sdata<f_section.subTree[subtree].subTree[subtree2].data.size();sdata++)
                                {
                                    if(f_section.subTree[subtree].subTree[subtree2].data[sdata].type!=PGEFile::PGEX_Struct)
                                    {
                                        errorString=QString("Wrong data item syntax:\nSubtree [%1]\nData line %2")
                                                .arg(f_section.subTree[subtree].subTree[subtree2].name)
                                                .arg(sdata);
                                        goto badfile;
                                    }

                                    PGEFile::PGEX_Item x = f_section.subTree[subtree].subTree[subtree2].data[sdata];

                                    QString name="";
                                    QString commandType="";
                                    int hx=0;
                                    int ft=0;
                                    double vf=0.0;

                                    //Get values
                                    for(int sval=0;sval<x.values.size();sval++) //Look markers and values
                                    {
                                        PGEFile::PGEX_Val v = x.values[sval];
                                        errorString=QString("Wrong value syntax\nSubtree [%1]\nData line %2\nMarker %3\nValue %4")
                                                .arg(f_section.subTree[subtree2].name)
                                                .arg(sdata)
                                                .arg(v.marker)
                                                .arg(v.value);

                                        if(v.marker=="N") //Command name
                                        {
                                            if(PGEFile::IsQoutedString(v.value))
                                                name = PGEFile::X2STRING(v.value);
                                            else
                                                goto badfile;
                                        }
                                        else
                                        if(v.marker=="CT") //Command type
                                        {
                                            if(PGEFile::IsQoutedString(v.value))
                                                commandType = PGEFile::X2STRING(v.value);
                                            else
                                                goto badfile;
                                        }


                                        //MemoryCommand specific values
                                        else
                                        if(v.marker=="HX") //Heximal value
                                        {
                                            if(PGEFile::IsIntS(v.value))
                                                hx = v.value.toInt();
                                            else
                                                goto badfile;
                                        }
                                        else
                                        if(v.marker=="FT") //Field type
                                        {
                                            if(PGEFile::IsIntS(v.value))
                                                ft = v.value.toInt();
                                            else
                                                goto badfile;
                                        }
                                        else
                                        if(v.marker=="V") //Value
                                        {
                                            if(PGEFile::IsFloat(v.value))
                                                vf = v.value.toDouble();
                                            else
                                                goto badfile;
                                        }
                                    }

                                    if(commandType=="MEMORY")
                                    {
                                        MemoryCommand *cmd =
                                           new MemoryCommand(hx,(MemoryCommand::FieldType)ft, vf);
                                        cmd->setMarker(name);
                                        event->addBasicCommand(cmd);
                                    }
                                }//commands list
                            }//Basic command subtree
                        }//subtrees
                    }

                    FileData.script->revents() << event;
                }//EVENT tree

            }//META_SCRIPT_EVENTS subtree
        }//META_SCRIPT_EVENTS Section
        #endif
    }
    ///////////////////////////////////////EndFile///////////////////////////////////////

    errorString.clear(); //If no errors, clear string;
    FileData.meta.ReadFileValid=true;
    return true;

    badfile:    //If file format is not correct
    //BadFileMsg(filePath+"\nError message: "+errorString, str_count, line);
    FileData.meta.ERROR_info=errorString;
    FileData.meta.ERROR_linenum=str_count;
    FileData.meta.ERROR_linedata=line;
    FileData.meta.ReadFileValid=false;

    FileData.bookmarks.clear();

    return false;
}






//*********************************************************
//****************WRITE FILE FORMAT************************
//*********************************************************

bool FileFormats::WriteNonSMBX64MetaDataF(PGESTRING filePath, MetaData &metaData)
{
    errorString.clear();
    PGE_FileFormats_misc::TextFileOutput file;
    if(!file.open(filePath, true, false, PGE_FileFormats_misc::TextOutput::truncate))
    {
        errorString="Failed to open file for write";
        return false;
    }
    return WriteNonSMBX64MetaData(file, metaData);
}

bool FileFormats::WriteNonSMBX64MetaDataRaw(MetaData &metaData, PGESTRING &rawdata)
{
    errorString.clear();
    PGE_FileFormats_misc::RawTextOutput file;
    if(!file.open(&rawdata, PGE_FileFormats_misc::TextOutput::truncate))
    {
        errorString="Failed to open raw string for write";
        return false;
    }
    return WriteNonSMBX64MetaData(file, metaData);
}

bool FileFormats::WriteNonSMBX64MetaData(PGE_FileFormats_misc::TextOutput &out, MetaData &metaData)
{
    long i;

    //Bookmarks
    if(!metaData.bookmarks.empty())
    {
        out << "META_BOOKMARKS\n";
        for(i=0;i<(signed)metaData.bookmarks.size(); i++)
        {
            //Bookmark name
            out << PGEFile::value("BM", PGEFile::WriteStr(metaData.bookmarks[i].bookmarkName));
            out << PGEFile::value("X", PGEFile::WriteInt(metaData.bookmarks[i].x));
            out << PGEFile::value("Y", PGEFile::WriteInt(metaData.bookmarks[i].y));
            out << "\n";
        }
        out << "META_BOOKMARKS_END\n";

        #ifdef PGE_EDITOR
        if(metaData.script)
        {
            if(!metaData.script->events().isEmpty())
            {
                out << "META_SCRIPT_EVENTS\n";
                foreach(EventCommand* x, metaData.script->events())
                {
                    out << "EVENT\n";
                    if(!x->marker().isEmpty())
                        out << PGEFile::value("TL", PGEFile::WriteStr( x->marker() ) );
                    out << PGEFile::value("ET", PGEFile::WriteInt( (int&)x->eventType() ) );
                    out << "\n";

                    if(x->basicCommands().size()>0)
                    {
                        out << "BASIC_COMMANDS\n";
                        foreach(BasicCommand *y, x->basicCommands())
                        {
                            out << PGEFile::value("N", PGEFile::WriteStr( y->marker() ) );
                            if(QString(y->metaObject()->className())=="MemoryCommand")
                            {
                                MemoryCommand *z = dynamic_cast<MemoryCommand*>(y);
                                out << PGEFile::value("CT", PGEFile::WriteStr( "MEMORY" ) );
                                out << PGEFile::value("HX", PGEFile::WriteInt( z->hexValue() ) );
                                out << PGEFile::value("FT", PGEFile::WriteInt( (int&)z->fieldType() ) );
                                out << PGEFile::value("V", PGEFile::WriteFloat( z->getValue() ) );
                            }
                            out << "\n";
                        }
                        out << "BASIC_COMMANDS_END\n";
                    }
                    out << "EVENT_END\n";
                }
                out << "META_SCRIPT_EVENTS_END\n";
            }
        }
        #endif
    }
    return true;
}

