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
#include "pge_x_macro.h"

#ifdef PGE_FILES_QT
#include <QDir>
#include <QFileInfo>
#endif

//*********************************************************
//****************READ FILE FORMAT*************************
//*********************************************************
bool FileFormats::ReadExtendedSaveFileF(PGESTRING filePath, GamesaveData &FileData)
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
    return ReadExtendedSaveFile(file, FileData);
}

bool FileFormats::ReadExtendedSaveFileRaw(PGESTRING &rawdata, PGESTRING filePath, GamesaveData &FileData)
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
    return ReadExtendedSaveFile(file, FileData);
}

bool FileFormats::ReadExtendedSaveFile(PGE_FileFormats_misc::TextInput &in, GamesaveData &FileData)
{
    FileData = CreateGameSaveData();
    PGESTRING errorString;
    PGEX_FileBegin();

    saveCharState plr_state;
    visibleItem        vz_item;
    starOnLevel        star_level;

    //Add path data
    PGESTRING fPath = in.getFilePath();
    if(!IsEmpty(fPath))
    {
        PGE_FileFormats_misc::FileInfo in_1(fPath);
        FileData.meta.filename = in_1.basename();
        FileData.meta.path = in_1.dirpath();
    }

    FileData.characterStates.clear();
    FileData.currentCharacter.clear();

    FileData.meta.untitled = false;
    FileData.meta.modified = false;

    ///////////////////////////////////////Begin file///////////////////////////////////////
    PGEX_FileParseTree(in.readAll());
    PGEX_FetchSection()
    {
        PGEX_FetchSection_begin()

        ///////////////////HEADER//////////////////////
        PGEX_Section("SAVE_HEADER")
        {
            PGEX_SectionBegin(PGEFile::PGEX_Struct);
            PGEX_Items()
            {
                PGEX_ItemBegin(PGEFile::PGEX_Struct);
                PGEX_Values() //Look markers and values
                {
                    PGEX_ValueBegin()
                    PGEX_UIntVal("LV", FileData.lives)
                    PGEX_UIntVal("CN", FileData.coins)
                    PGEX_UIntVal("PT", FileData.points)
                    PGEX_UIntVal("TS", FileData.totalStars)
                    PGEX_SIntVal("WX", FileData.worldPosX)
                    PGEX_SIntVal("WY", FileData.worldPosY)
                    PGEX_SIntVal("HW", FileData.last_hub_warp)
                    PGEX_UIntVal("MI", FileData.musicID)
                    PGEX_StrVal ("MF", FileData.musicFile)
                    PGEX_BoolVal("GC", FileData.gameCompleted)
                }
            }
        }//Header


        ///////////////////CHARACTERS//////////////////////
        PGEX_Section("CHARACTERS")
        {
            PGEX_SectionBegin(PGEFile::PGEX_Struct);
            PGEX_Items()
            {
                PGEX_ItemBegin(PGEFile::PGEX_Struct);
                plr_state = CreateSavCharacterState();
                PGEX_Values()
                {
                    PGEX_ValueBegin()
                    PGEX_UIntVal("ID", plr_state.id)
                    PGEX_UIntVal("ST", plr_state.state)
                    PGEX_UIntVal("IT", plr_state.itemID)
                    PGEX_UIntVal("MT", plr_state.mountType)
                    PGEX_UIntVal("MI", plr_state.mountID)
                    PGEX_UIntVal("HL", plr_state.health)
                }
                FileData.characterStates.push_back(plr_state);
            }
        }//CHARACTERS


        ///////////////////CHARACTERS_PER_PLAYERS//////////////////////
        PGEX_Section("CHARACTERS_PER_PLAYERS")
        {
            PGEX_SectionBegin(PGEFile::PGEX_Struct);
            PGEX_Items()
            {
                PGEX_ItemBegin(PGEFile::PGEX_Struct);
                int character=0;
                PGEX_Values()
                {
                    PGEX_ValueBegin()
                    PGEX_UIntVal("ID", character)
                }
                FileData.currentCharacter.push_back(character);
            }
        }//CHARACTERS_PER_PLAYERS

        ///////////////////VIZ_LEVELS//////////////////////
        PGEX_Section("VIZ_LEVELS")
        {
            PGEX_SectionBegin(PGEFile::PGEX_Struct);
            PGEX_Items()
            {
                PGEX_ItemBegin(PGEFile::PGEX_Struct);

                vz_item.first=0;
                vz_item.second=false;
                PGEX_Values() //Look markers and values
                {
                    PGEX_ValueBegin()
                    PGEX_UIntVal("ID", vz_item.first)
                    PGEX_BoolVal("V", vz_item.second)
                }
                FileData.visibleLevels.push_back(vz_item);
            }
        }//VIZ_LEVELS

        ///////////////////VIZ_PATHS//////////////////////
        PGEX_Section("VIZ_PATHS")
        {
            PGEX_SectionBegin(PGEFile::PGEX_Struct);
            PGEX_Items()
            {
                PGEX_ItemBegin(PGEFile::PGEX_Struct);

                vz_item.first=0;
                vz_item.second=false;
                PGEX_Values() //Look markers and values
                {
                    PGEX_ValueBegin()
                    PGEX_UIntVal("ID", vz_item.first)
                    PGEX_BoolVal("V", vz_item.second)
                }
                FileData.visiblePaths.push_back(vz_item);
            }
        }//VIZ_PATHS

        ///////////////////VIZ_SCENERY//////////////////////
        PGEX_Section("VIZ_SCENERY")
        {
            PGEX_SectionBegin(PGEFile::PGEX_Struct);
            PGEX_Items()
            {
                PGEX_ItemBegin(PGEFile::PGEX_Struct);

                vz_item.first=0;
                vz_item.second=false;
                PGEX_Values() //Look markers and values
                {
                    PGEX_ValueBegin()
                    PGEX_UIntVal("ID", vz_item.first)
                    PGEX_BoolVal("V", vz_item.second)
                }
                FileData.visibleScenery.push_back(vz_item);
            }
        }//VIZ_SCENERY

        ///////////////////STARS//////////////////////
        PGEX_Section("STARS")
        {
            PGEX_SectionBegin(PGEFile::PGEX_Struct);
            PGEX_Items()
            {
                PGEX_ItemBegin(PGEFile::PGEX_Struct);

                star_level.first="";
                star_level.second=0;
                PGEX_Values() //Look markers and values
                {
                    PGEX_ValueBegin()
                    PGEX_StrVal("L", star_level.first)
                    PGEX_UIntVal("S", star_level.second)
                }
                FileData.gottenStars.push_back(star_level);
            }
        }//STARS
    }
    ///////////////////////////////////////EndFile///////////////////////////////////////
    errorString.clear(); //If no errors, clear string;
    FileData.meta.ReadFileValid=true;

    return true;

badfile:    //If file format not corrects
    FileData.meta.ERROR_info=errorString;
    FileData.meta.ERROR_linenum=str_count;
    FileData.meta.ERROR_linedata=line;
    FileData.meta.ReadFileValid=false;
    return false;
}


//*********************************************************
//****************WRITE FILE FORMAT************************
//*********************************************************

bool FileFormats::WriteExtendedSaveFileF(PGESTRING filePath, GamesaveData &FileData)
{
    errorString.clear();
    PGE_FileFormats_misc::TextFileOutput file;
    if(!file.open(filePath, true, false, PGE_FileFormats_misc::TextOutput::truncate))
    {
        errorString="Failed to open file for write";
        return false;
    }
    return WriteExtendedSaveFile(file, FileData);
}

bool FileFormats::WriteExtendedSaveFileRaw(GamesaveData &FileData, PGESTRING &rawdata)
{
    errorString.clear();
    PGE_FileFormats_misc::RawTextOutput file;
    if(!file.open(&rawdata, PGE_FileFormats_misc::TextOutput::truncate))
    {
        errorString="Failed to open raw string for write";
        return false;
    }
    return WriteExtendedSaveFile(file, FileData);
}

bool FileFormats::WriteExtendedSaveFile(PGE_FileFormats_misc::TextOutput &out, GamesaveData &FileData)
{
    long i;

    out << "SAVE_HEADER\n";
    out << PGEFile::value("LV", PGEFile::WriteInt(FileData.lives));
    out << PGEFile::value("CN", PGEFile::WriteInt(FileData.coins));
    out << PGEFile::value("PT", PGEFile::WriteInt(FileData.points));
    out << PGEFile::value("TS", PGEFile::WriteInt(FileData.totalStars));
    out << PGEFile::value("WX", PGEFile::WriteInt(FileData.worldPosX));
    out << PGEFile::value("WY", PGEFile::WriteInt(FileData.worldPosY));
    out << PGEFile::value("HW", PGEFile::WriteInt(FileData.last_hub_warp));
    out << PGEFile::value("MI", PGEFile::WriteInt(FileData.musicID));
    out << PGEFile::value("MF", PGEFile::WriteStr(FileData.musicFile));
    out << PGEFile::value("GC", PGEFile::WriteBool(FileData.gameCompleted));
    out << "\n";
    out << "SAVE_HEADER_END\n";

    if(!FileData.characterStates.empty())
    {
        out << "CHARACTERS\n";
        for(i=0;i< (signed)FileData.characterStates.size(); i++)
        {
            out << PGEFile::value("ID", PGEFile::WriteInt(FileData.characterStates[i].id));
            out << PGEFile::value("ST", PGEFile::WriteInt(FileData.characterStates[i].state));
            out << PGEFile::value("IT", PGEFile::WriteInt(FileData.characterStates[i].itemID));
            out << PGEFile::value("MT", PGEFile::WriteInt(FileData.characterStates[i].mountType));
            out << PGEFile::value("MI", PGEFile::WriteInt(FileData.characterStates[i].mountID));
            out << PGEFile::value("HL", PGEFile::WriteInt(FileData.characterStates[i].health));
            out << "\n";
        }
        out << "CHARACTERS_END\n";
    }

    if(!FileData.currentCharacter.empty())
    {
        out << "CHARACTERS_PER_PLAYERS\n";
        for(i=0;i< (signed)FileData.currentCharacter.size(); i++)
        {
            out << PGEFile::value("ID", PGEFile::WriteInt(FileData.currentCharacter[i]));
            out << "\n";
        }
        out << "CHARACTERS_PER_PLAYERS_END\n";
    }

    if(!FileData.visibleLevels.empty())
    {
        out << "VIZ_LEVELS\n";
        for(i=0;i< (signed)FileData.visibleLevels.size(); i++)
        {
            out << PGEFile::value("ID", PGEFile::WriteInt(FileData.visibleLevels[i].first));
            out << PGEFile::value("V", PGEFile::WriteBool(FileData.visibleLevels[i].second));
            out << "\n";
        }
        out << "VIZ_LEVELS_END\n";
    }

    if(!FileData.visiblePaths.empty())
    {
        out << "VIZ_PATHS\n";
        for(i=0;i< (signed)FileData.visiblePaths.size(); i++)
        {
            out << PGEFile::value("ID", PGEFile::WriteInt(FileData.visiblePaths[i].first));
            out << PGEFile::value("V", PGEFile::WriteBool(FileData.visiblePaths[i].second));
            out << "\n";
        }
        out << "VIZ_PATHS_END\n";
    }

    if(!FileData.visibleScenery.empty())
    {
        out << "VIZ_SCENERY\n";
        for(i=0;i< (signed)FileData.visibleScenery.size(); i++)
        {
            out << PGEFile::value("ID", PGEFile::WriteInt(FileData.visibleScenery[i].first));
            out << PGEFile::value("V", PGEFile::WriteBool(FileData.visibleScenery[i].second));
            out << "\n";
        }
        out << "VIZ_SCENERY_END\n";
    }

    if(!FileData.gottenStars.empty())
    {
        out << "STARS\n";
        for(i=0;i< (signed)FileData.gottenStars.size(); i++)
        {
            out << PGEFile::value("L", PGEFile::WriteStr(FileData.gottenStars[i].first));
            out << PGEFile::value("S", PGEFile::WriteInt(FileData.gottenStars[i].second));
            out << "\n";
        }
        out << "STARS_END\n";
    }

    out << "\n";
    return true;
}


