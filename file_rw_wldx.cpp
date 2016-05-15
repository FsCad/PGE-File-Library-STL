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
#include "wld_filedata.h"
#include "pge_x.h"
#include "pge_x_macro.h"
#include "pge_file_lib_sys.h"

#ifdef PGE_FILES_USE_MESSAGEBOXES
#include <QMessageBox>
#endif


//*********************************************************
//****************READ FILE FORMAT*************************
//*********************************************************

//WorldData FileFormats::ReadExtendedWorldFile(PGEFILE &inf)
//{
//    QTextStream in(&inf);   //Read File
//    in.setCodec("UTF-8");

//    return ReadExtendedWldFile( in.readAll(), inf.fileName() );
//}

bool FileFormats::ReadExtendedWldFileHeader(PGESTRING filePath, WorldData &FileData)
{
    CreateWorldHeader(FileData);
    FileData.RecentFormat = WorldData::PGEX;

    PGE_FileFormats_misc::TextFileInput  inf;
    if(!inf.open(filePath, true))
    {
        FileData.ReadFileValid=false;
        return false;
    }

    PGESTRING line;
    int str_count=0;
    bool valid=false;
    PGE_FileFormats_misc::FileInfo in_1(filePath);
    FileData.filename = in_1.basename();
    FileData.path = in_1.dirpath();

    //Find level header part
    do{
    str_count++;line = inf.readLine();
    }while((line!="HEAD") && (!inf.eof()));

    PGESTRINGList header;
    str_count++;line = inf.readLine();
    bool closed=false;
    while((line!="HEAD_END") && (!inf.eof()))
    {
        header.push_back(line);
        str_count++;line = inf.readLine();
        if(line=="HEAD_END") closed=true;
    }
    if(!closed) goto badfile;

    for(int zzz=0;zzz<(signed)header.size();zzz++)
    {
        PGESTRING &header_line=header[zzz];
        PGELIST<PGESTRINGList >data = PGEFile::splitDataLine(header_line, &valid);

        for(int i=0;i<(signed)data.size();i++)
        {
            if(data[i].size()!=2) goto badfile;
            if(data[i][0]=="TL") //Episode Title
            {
             if(PGEFile::IsQoutedString(data[i][1]))
                 FileData.EpisodeTitle = PGEFile::X2STRING(data[i][1]);
             else
                 goto badfile;
            }
            else
            if(data[i][0]=="DC") //Disabled characters
            {
             if(PGEFile::IsBoolArray(data[i][1]))
                 FileData.nocharacter = PGEFile::X2BollArr(data[i][1]);
             else
                 goto badfile;
            }
            else
            if(data[i][0]=="IT") //Intro level
            {
             if(PGEFile::IsQoutedString(data[i][1]))
                 FileData.IntroLevel_file = PGEFile::X2STRING(data[i][1]);
             else
                 goto badfile;
            }
            else
            if(data[i][0]=="HB") //Hub Styled
            {
             if(PGEFile::IsBool(data[i][1]))
                 FileData.HubStyledWorld = (bool)toInt(data[i][1]);
             else
                 goto badfile;
            }
            else
            if(data[i][0]=="RL") //Restart level on fail
            {
             if(PGEFile::IsBool(data[i][1]))
                 FileData.restartlevel = (bool)toInt(data[i][1]);
             else
                 goto badfile;
            }
            else
            if(data[i][0]=="SZ") //Starz number
            {
             if(PGEFile::IsIntU(data[i][1]))
                 FileData.stars = toInt(data[i][1]);
             else
                 goto badfile;
            }
            else
            if(data[i][0]=="CD") //Credits list
            {
             if(PGEFile::IsQoutedString(data[i][1]))
                 FileData.authors = PGEFile::X2STRING(data[i][1]);
             else
                 goto badfile;
            }
        }
    }

    if(!closed)
        goto badfile;

    FileData.CurSection=0;
    FileData.playmusic=0;

    FileData.ReadFileValid=true;

    inf.close();
    return true;

badfile:
    inf.close();
    FileData.ERROR_info="Invalid file format";
    FileData.ERROR_linenum=str_count;
    FileData.ERROR_linedata=line;
    FileData.ReadFileValid=false;
    return false;
}

bool FileFormats::ReadExtendedWldFileF(PGESTRING  filePath, WorldData &FileData)
{
    errorString.clear();
    PGE_FileFormats_misc::TextFileInput file;
    if(!file.open(filePath, true))
    {
        errorString="Failed to open file for read";
        FileData.ERROR_info = errorString;
        FileData.ERROR_linedata = "";
        FileData.ERROR_linenum = -1;
        FileData.ReadFileValid = false;
        return false;
    }
    return ReadExtendedWldFile(file, FileData);
}

bool FileFormats::ReadExtendedWldFileRaw(PGESTRING &rawdata, PGESTRING  filePath,  WorldData &FileData)
{
    errorString.clear();
    PGE_FileFormats_misc::RawTextInput file;
    if(!file.open(&rawdata, filePath))
    {
        errorString="Failed to open raw string for read";
        FileData.ERROR_info = errorString;
        FileData.ERROR_linedata = "";
        FileData.ERROR_linenum = -1;
        FileData.ReadFileValid = false;
        return false;
    }
    return ReadExtendedWldFile(file, FileData);
}

bool FileFormats::ReadExtendedWldFile(PGE_FileFormats_misc::TextInput &in, WorldData &FileData)
{
     PGESTRING errorString;
     PGEX_FileBegin();

     PGESTRING filePath = in.getFilePath();

     CreateWorldData(FileData);
     FileData.RecentFormat = WorldData::PGEX;

     //Add path data
     if(!IsEmpty(filePath))
     {
         PGE_FileFormats_misc::FileInfo in_1(filePath);
         FileData.filename = in_1.basename();
         FileData.path = in_1.dirpath();
     }

     FileData.untitled = false;
     FileData.modified = false;

     WorldTiles tile;
     WorldScenery scen;
     WorldPaths pathitem;
     WorldMusic musicbox;
     WorldLevels lvlitem;

     ///////////////////////////////////////Begin file///////////////////////////////////////
     PGEX_FileParseTree(in.readAll());

     PGEX_FetchSection() //look sections
     {
         PGEX_FetchSection_begin()

         ///////////////////HEADER//////////////////////
         PGEX_Section("HEAD")
         {
             str_count++;
             PGEX_SectionBegin(PGEFile::PGEX_Struct);
             PGEX_Items()
             {
                 str_count+=8;
                 PGEX_ItemBegin(PGEFile::PGEX_Struct);
                 PGEX_Values() //Look markers and values
                 {
                     PGEX_ValueBegin()
                     PGEX_StrVal    ("TL", FileData.EpisodeTitle)    //Episode Title
                     PGEX_BoolArrVal("DC", FileData.nocharacter)     //Disabled characters
                     PGEX_StrVal    ("IT", FileData.IntroLevel_file) //Intro level
                     PGEX_BoolVal   ("HB", FileData.HubStyledWorld)  //Hub Styled
                     PGEX_BoolVal   ("RL", FileData.restartlevel)    //Restart level on fail
                     PGEX_UIntVal   ("SZ", FileData.stars)           //Starz number
                     PGEX_StrVal    ("CD", FileData.authors) //Credits list
                 }
             }
         }//head


         ///////////////////////////////MetaDATA/////////////////////////////////////////////
         PGEX_Section("META_BOOKMARKS")
         {
             str_count++;
             PGEX_SectionBegin(PGEFile::PGEX_Struct);

             PGEX_Items()
             {
                 str_count++;
                 PGEX_ItemBegin(PGEFile::PGEX_Struct);

                 Bookmark meta_bookmark;
                 meta_bookmark.bookmarkName = "";
                 meta_bookmark.x = 0;
                 meta_bookmark.y = 0;

                 PGEX_Values() //Look markers and values
                 {
                     PGEX_ValueBegin()
                     PGEX_StrVal("BM", meta_bookmark.bookmarkName) //Bookmark name
                     PGEX_SIntVal("X", meta_bookmark.x) // Position X
                     PGEX_SIntVal("Y", meta_bookmark.y) // Position Y
                 }
                 FileData.metaData.bookmarks.push_back(meta_bookmark);
             }
         }

         ////////////////////////meta bookmarks////////////////////////
         #ifdef PGE_EDITOR
         PGEX_Section("META_SYS_CRASH")
         {
             str_count++;
             PGEX_SectionBegin(PGEFile::PGEX_Struct);

             PGEX_Items()
             {
                 str_count++;
                 PGEX_ItemBegin(PGEFile::PGEX_Struct);

                 PGEX_Values() //Look markers and values
                 {
                     FileData.metaData.crash.used=true;

                     PGEX_ValueBegin()
                     PGEX_BoolVal("UT", FileData.metaData.crash.untitled) //Untitled
                     PGEX_BoolVal("MD", FileData.metaData.crash.modifyed) //Modyfied
                     PGEX_StrVal ("N",  FileData.metaData.crash.filename) //Filename
                     PGEX_StrVal ("P",  FileData.metaData.crash.path) //Path
                     PGEX_StrVal ("FP", FileData.metaData.crash.fullPath) //Full file Path
                 }
             }
         }//meta sys crash
         #endif
         ///////////////////////////////MetaDATA//End////////////////////////////////////////


         ///////////////////TILES//////////////////////
         PGEX_Section("TILES")
         {
             str_count++;
             PGEX_SectionBegin(PGEFile::PGEX_Struct);

             PGEX_Items()
             {
                 str_count++;
                 PGEX_ItemBegin(PGEFile::PGEX_Struct);
                 tile = CreateWldTile();

                 PGEX_Values() //Look markers and values
                 {
                     PGEX_ValueBegin()
                     PGEX_UIntVal("ID", tile.id) //Tile ID
                     PGEX_SIntVal("X",  tile.x) //X Position
                     PGEX_SIntVal("Y",  tile.y) //Y Position
                 }

                 tile.array_id = FileData.tile_array_id++;
                 tile.index = FileData.tiles.size();
                 FileData.tiles.push_back(tile);
             }
         }//TILES


         ///////////////////SCENERY//////////////////////
         PGEX_Section("SCENERY")
         {
             str_count++;
             PGEX_SectionBegin(PGEFile::PGEX_Struct);

             PGEX_Items()
             {
                 str_count++;
                 PGEX_ItemBegin(PGEFile::PGEX_Struct);
                 scen = CreateWldScenery();

                 PGEX_Values() //Look markers and values
                 {
                     PGEX_ValueBegin()
                     PGEX_UIntVal("ID", scen.id ) //Scenery ID
                     PGEX_SIntVal("X", scen.x) //X Position
                     PGEX_SIntVal("Y", scen.y) //Y Position
                 }

                 scen.array_id = FileData.scene_array_id++;
                 scen.index = FileData.scenery.size();
                 FileData.scenery.push_back(scen);
             }
         }//SCENERY


         ///////////////////PATHS//////////////////////
         PGEX_Section("PATHS")
         {
             str_count++;
             PGEX_SectionBegin(PGEFile::PGEX_Struct);

             PGEX_Items()
             {
                 str_count++;
                 PGEX_ItemBegin(PGEFile::PGEX_Struct);
                 pathitem = CreateWldPath();

                 PGEX_Values() //Look markers and values
                 {
                     PGEX_ValueBegin()
                     PGEX_UIntVal("ID", pathitem.id ) //Path ID
                     PGEX_SIntVal("X", pathitem.x) //X Position
                     PGEX_SIntVal("Y", pathitem.y) //Y Position
                 }
                 pathitem.array_id = FileData.path_array_id++;
                 pathitem.index = FileData.paths.size();
                 FileData.paths.push_back(pathitem);
             }
         }//PATHS


         ///////////////////MUSICBOXES//////////////////////
         PGEX_Section("MUSICBOXES")
         {
             str_count++;
             PGEX_SectionBegin(PGEFile::PGEX_Struct);

             PGEX_Items()
             {
                 str_count++;
                 PGEX_ItemBegin(PGEFile::PGEX_Struct);
                 musicbox = CreateWldMusicbox();

                 PGEX_Values() //Look markers and values
                 {
                     PGEX_ValueBegin()
                     PGEX_UIntVal("ID", musicbox.id) //MISICBOX ID
                     PGEX_SIntVal("X", musicbox.x) //X Position
                     PGEX_SIntVal("Y", musicbox.y) //X Position
                     PGEX_StrVal ("MF", musicbox.music_file) //Custom music file
                 }
                 musicbox.array_id = FileData.musicbox_array_id++;
                 musicbox.index = FileData.music.size();
                 FileData.music.push_back(musicbox);
             }
         }//MUSICBOXES

         ///////////////////LEVELS//////////////////////
         PGEX_Section("LEVELS")
         {
             str_count++;
             PGEX_SectionBegin(PGEFile::PGEX_Struct);

             PGEX_Items()
             {
                 str_count++;
                 PGEX_ItemBegin(PGEFile::PGEX_Struct);

                 lvlitem = CreateWldLevel();
                 PGEX_Values() //Look markers and values
                 {
                     PGEX_ValueBegin()
                     PGEX_UIntVal("ID", lvlitem.id) //LEVEL IMAGE ID
                     PGEX_SIntVal("X",  lvlitem.x) //X Position
                     PGEX_SIntVal("Y",  lvlitem.y) //X Position
                     PGEX_StrVal ("LF", lvlitem.lvlfile) //Target level file
                     PGEX_StrVal ("LT", lvlitem.title)  //Level title
                     PGEX_UIntVal("EI", lvlitem.entertowarp) //Entrance Warp ID (if 0 - start level from default points)
                     PGEX_SIntVal("ET", lvlitem.top_exit) //Open top path on exit type
                     PGEX_SIntVal("EL", lvlitem.left_exit) //Open left path on exit type
                     PGEX_SIntVal("ER", lvlitem.right_exit) //Open right path on exit type
                     PGEX_SIntVal("EB", lvlitem.bottom_exit) //Open bottom path on exit type
                     PGEX_SIntVal("WX", lvlitem.gotox) //Goto world map X
                     PGEX_SIntVal("WY", lvlitem.gotoy) //Goto world map Y
                     PGEX_BoolVal("AV", lvlitem.alwaysVisible) //Always visible
                     PGEX_BoolVal("SP", lvlitem.gamestart) //Is Game start point
                     PGEX_BoolVal("BP", lvlitem.pathbg) //Path background
                     PGEX_BoolVal("BG", lvlitem.bigpathbg) //Big path background
                 }
                 lvlitem.array_id = FileData.level_array_id++;
                 lvlitem.index = FileData.levels.size();
                 FileData.levels.push_back(lvlitem);
             }
         }//LEVELS

     }
     ///////////////////////////////////////EndFile///////////////////////////////////////

     errorString.clear(); //If no errors, clear string;
     FileData.ReadFileValid=true;

     return true;

     badfile:    //If file format not corrects
         FileData.ERROR_info=errorString;
         FileData.ERROR_linenum=str_count;
         FileData.ERROR_linedata=line;

         FileData.ReadFileValid=false;

     return false;
}



//*********************************************************
//****************WRITE FILE FORMAT************************
//*********************************************************

bool FileFormats::WriteExtendedWldFileF(PGESTRING filePath, WorldData &FileData)
{
    errorString.clear();
    PGE_FileFormats_misc::TextFileOutput file;
    if(!file.open(filePath, true, false, PGE_FileFormats_misc::TextOutput::truncate))
    {
        errorString="Failed to open file for write";
        return false;
    }
    return WriteExtendedWldFile(file, FileData);
}

bool FileFormats::WriteExtendedWldFileRaw(WorldData &FileData, PGESTRING &rawdata)
{
    errorString.clear();
    PGE_FileFormats_misc::RawTextOutput file;
    if(!file.open(&rawdata, PGE_FileFormats_misc::TextOutput::truncate))
    {
        errorString="Failed to open raw string for write";
        return false;
    }
    return WriteExtendedWldFile(file, FileData);
}

bool FileFormats::WriteExtendedWldFile(PGE_FileFormats_misc::TextOutput &out, WorldData &FileData)
{
    long i;
    bool addArray=false;

    FileData.RecentFormat = WorldData::PGEX;

    addArray=false;
    for(int z=0; z<(signed)FileData.nocharacter.size();z++)
    { bool x=FileData.nocharacter[z]; if(x) addArray=true; }
    //HEAD section
    if(
            (!IsEmpty(FileData.EpisodeTitle))||
            (addArray)||
            (!IsEmpty(FileData.IntroLevel_file))||
            (FileData.HubStyledWorld)||
            (FileData.restartlevel)||
            (FileData.stars>0)||
            (!IsEmpty(FileData.authors))
      )
    {
        out << "HEAD\n";
            if(!IsEmpty(FileData.EpisodeTitle))
                out << PGEFile::value("TL", PGEFile::WriteStr(FileData.EpisodeTitle)); // Episode title

            addArray=false;
            for(int z=0; z<(signed)FileData.nocharacter.size();z++)
            { bool x=FileData.nocharacter[z]; if(x) addArray=true; }
            if(addArray)
                out << PGEFile::value("DC", PGEFile::WriteBoolArr(FileData.nocharacter)); // Disabled characters

            if(!IsEmpty(FileData.IntroLevel_file))
                out << PGEFile::value("IT", PGEFile::WriteStr(FileData.IntroLevel_file)); // Intro level

            if(FileData.HubStyledWorld)
                out << PGEFile::value("HB", PGEFile::WriteBool(FileData.HubStyledWorld)); // Hub-styled episode

            if(FileData.restartlevel)
                out << PGEFile::value("RL", PGEFile::WriteBool(FileData.restartlevel)); // Restart on fail
            if(FileData.stars>0)
                out << PGEFile::value("SZ", PGEFile::WriteInt(FileData.stars));      // Total stars number
            if(!IsEmpty(FileData.authors))
                out << PGEFile::value("CD", PGEFile::WriteStr( FileData.authors )); // Credits

        out << "\n";
        out << "HEAD_END\n";
    }

    //////////////////////////////////////MetaData////////////////////////////////////////////////
    //Bookmarks
    if(!FileData.metaData.bookmarks.empty())
    {
        out << "META_BOOKMARKS\n";
        for(i=0;i<(signed)FileData.metaData.bookmarks.size(); i++)
        {
            //Bookmark name
            out << PGEFile::value("BM", PGEFile::WriteStr(FileData.metaData.bookmarks[i].bookmarkName));
            out << PGEFile::value("X", PGEFile::WriteInt(FileData.metaData.bookmarks[i].x));
            out << PGEFile::value("Y", PGEFile::WriteInt(FileData.metaData.bookmarks[i].y));
            out << "\n";
        }
        out << "META_BOOKMARKS_END\n";
    }

    #ifdef PGE_EDITOR
    //Some System information
    if(FileData.metaData.crash.used)
    {
        out << "META_SYS_CRASH\n";
            out << PGEFile::value("UT", PGEFile::WriteBool(FileData.metaData.crash.untitled));
            out << PGEFile::value("MD", PGEFile::WriteBool(FileData.metaData.crash.modifyed));
            out << PGEFile::value("N", PGEFile::WriteStr(FileData.metaData.crash.filename));
            out << PGEFile::value("P", PGEFile::WriteStr(FileData.metaData.crash.path));
            out << PGEFile::value("FP", PGEFile::WriteStr(FileData.metaData.crash.fullPath));
            out << "\n";
        out << "META_SYS_CRASH_END\n";
    }
    #endif
    //////////////////////////////////////MetaData///END//////////////////////////////////////////

    if(!FileData.tiles.empty())
    {
        out << "TILES\n";

        for(i=0; i<(signed)FileData.tiles.size();i++)
        {
            out << PGEFile::value("ID", PGEFile::WriteInt(FileData.tiles[i].id ));
            out << PGEFile::value("X", PGEFile::WriteInt(FileData.tiles[i].x ));
            out << PGEFile::value("Y", PGEFile::WriteInt(FileData.tiles[i].y ));
            out << "\n";
        }

        out << "TILES_END\n";
    }

    if(!FileData.scenery.empty())
    {
        out << "SCENERY\n";

        for(i=0; i<(signed)FileData.scenery.size();i++)
        {
            out << PGEFile::value("ID", PGEFile::WriteInt(FileData.scenery[i].id ));
            out << PGEFile::value("X", PGEFile::WriteInt(FileData.scenery[i].x ));
            out << PGEFile::value("Y", PGEFile::WriteInt(FileData.scenery[i].y ));
            out << "\n";
        }

        out << "SCENERY_END\n";
    }

    if(!FileData.paths.empty())
    {
        out << "PATHS\n";

        for(i=0; i<(signed)FileData.paths.size();i++)
        {
            out << PGEFile::value("ID", PGEFile::WriteInt(FileData.paths[i].id ));
            out << PGEFile::value("X", PGEFile::WriteInt(FileData.paths[i].x ));
            out << PGEFile::value("Y", PGEFile::WriteInt(FileData.paths[i].y ));
            out << "\n";
        }

        out << "PATHS_END\n";
    }

    if(!FileData.music.empty())
    {
        out << "MUSICBOXES\n";

        for(i=0; i<(signed)FileData.music.size();i++)
        {
            out << PGEFile::value("ID", PGEFile::WriteInt(FileData.music[i].id ));
            out << PGEFile::value("X", PGEFile::WriteInt(FileData.music[i].x ));
            out << PGEFile::value("Y", PGEFile::WriteInt(FileData.music[i].y ));
            if(!IsEmpty(FileData.music[i].music_file))
                out << PGEFile::value("MF", PGEFile::WriteStr(FileData.music[i].music_file ));
            out << "\n";
        }

        out << "MUSICBOXES_END\n";
    }


    if(!FileData.levels.empty())
    {
        out << "LEVELS\n";

        WorldLevels defLvl = CreateWldLevel();
        for(i=0; i<(signed)FileData.levels.size();i++)
        {
            out << PGEFile::value("ID", PGEFile::WriteInt(FileData.levels[i].id ));
            out << PGEFile::value("X", PGEFile::WriteInt(FileData.levels[i].x ));
            out << PGEFile::value("Y", PGEFile::WriteInt(FileData.levels[i].y ));
            if(!IsEmpty(FileData.levels[i].title))
                out << PGEFile::value("LT", PGEFile::WriteStr(FileData.levels[i].title ));
            if(!IsEmpty(FileData.levels[i].lvlfile))
                out << PGEFile::value("LF", PGEFile::WriteStr(FileData.levels[i].lvlfile ));
            if(FileData.levels[i].entertowarp!=defLvl.entertowarp)
                out << PGEFile::value("EI", PGEFile::WriteInt(FileData.levels[i].entertowarp ));
            if(FileData.levels[i].left_exit!=defLvl.left_exit)
                out << PGEFile::value("EL", PGEFile::WriteInt(FileData.levels[i].left_exit ));
            if(FileData.levels[i].top_exit!=defLvl.top_exit)
                out << PGEFile::value("ET", PGEFile::WriteInt(FileData.levels[i].top_exit ));
            if(FileData.levels[i].right_exit!=defLvl.right_exit)
                out << PGEFile::value("ER", PGEFile::WriteInt(FileData.levels[i].right_exit ));
            if(FileData.levels[i].bottom_exit!=defLvl.bottom_exit)
                out << PGEFile::value("EB", PGEFile::WriteInt(FileData.levels[i].bottom_exit ));
            if(FileData.levels[i].gotox!=defLvl.gotox)
                out << PGEFile::value("WX", PGEFile::WriteInt(FileData.levels[i].gotox ));
            if(FileData.levels[i].gotoy!=defLvl.gotoy)
                out << PGEFile::value("WY", PGEFile::WriteInt(FileData.levels[i].gotoy ));
            if(FileData.levels[i].alwaysVisible)
                out << PGEFile::value("AV", PGEFile::WriteBool(FileData.levels[i].alwaysVisible ));
            if(FileData.levels[i].gamestart)
                out << PGEFile::value("SP", PGEFile::WriteBool(FileData.levels[i].gamestart ));
            if(FileData.levels[i].pathbg)
                out << PGEFile::value("BP", PGEFile::WriteBool(FileData.levels[i].pathbg ));
            if(FileData.levels[i].bigpathbg)
                out << PGEFile::value("BG", PGEFile::WriteBool(FileData.levels[i].bigpathbg ));
            out << "\n";
        }

        out << "LEVELS_END\n";
    }
    return true;
}
