#include "MiniMenuForEveryone.hpp"
#include "cheats.hpp"
#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework
{

    //プロトタイプ宣言
    bool DrawMainMenu   (const Screen &screen);

    //終わりの部分を指定して描写
    void DrawBackword(const Screen &screen, std::string &str, u32 posX, u32 posY, const Color &color)
    {
        int VertextWidth = OSD::GetTextWidth(true, str);    //文字列の長さ取得
        screen.DrawSysfont(str, (posX - VertextWidth), posY, color);
    }

    //改行できるSysfont
    void DrawSysfont_newLinePlus(const Screen &screen, const std::string &str, u32 posX, u32 posY, const Color &color)
    {
        const int gap = 12;
        
        std::vector<std::pair<int, int>> LineRange;
        int start = 0;
        for (int i = 0; i < str.length(); i++)
        {
            if (str[i] == '\n')
            {
                LineRange.emplace_back(start, i); // i - 1 ではなく i を使う
                start = i + 1;
            }
        }
        LineRange.emplace_back(start, str.length());

        for (int i = 0; i < LineRange.size(); i++)
        {
            screen.DrawSysfont(str.substr(LineRange[i].first, LineRange[i].second - LineRange[i].first), posX, posY + i * 12, color);
        }
    }

    struct Cheat_Item{
        //UserSetting
        int             type;                       //エントリータイプ
        std::string     name, note;                 //名前, ノート
        u32             Address, Value;             //アドレス, 値
        //Another
        bool            Information     = false;    //ノートの有無
        bool            ProgramRunning  = false;    //プログラムがうごいてるか(type 1のみ)
    };

    namespace MiniMenuOverview
    {
        std::string             TitleName, VerString;   //name, version
        std::vector<Cheat_Item> Entry;                  //概要
        static bool Cheating            = false;        //一つでもチートがオンか
        static bool InformationOpened   = false;        //(i)を開いてるか
        std::string             Copyright = "Basic developer: FPS 18";
    }

    // type 0:チェックタイプ    1:チェック不可
    void new_entry (const std::string &name, short type, u32 Address, u32 Value, const std::string &note)
    {
        //重複確認
        bool isDuplicate = false;
        for (const Cheat_Item &entry : MiniMenuOverview::Entry)
        {
            if (entry.name == name && entry.type == type && entry.Address == Address && entry.Value == Value && entry.note == note)
            {
                isDuplicate = true;
                break;
            }
        }
        //名前の長さチェック

        if (!isDuplicate)
        {
            Cheat_Item newItem;
            newItem.name    =   name;
            newItem.note    =   note;
            newItem.type    =   type;
            newItem.Address =   Address;
            newItem.Value   =   Value;
            //Informationの有無
            if (note != "")
            {
                newItem.Information = true;
            }

            MiniMenuOverview::Entry.push_back(newItem);
        }
    }


    //const std::string str = title
    //const char ver = version
    //u32 Button = Buttons to display on the display
    void Implementation_MiniMenu (const std::string &str, u32 major, u32 minor, u32 revision, u32 Button)
    {    
        MiniMenuOverview::TitleName = str;  //タイトルの名前代入
        static bool MenuOpened = false;         //メニューが開いてるか判別するやつ
                                                //staticにすると何回読み込んでも上書きされない
        //ver代入
        MiniMenuOverview::VerString = "[" + Utils::Format("%d.%d.%d", major, minor, revision) + "]";

        //メニューのOnOff
        if (Controller::IsKeysPressed(Button))
        {
            //メニューが開いてる
            if (MenuOpened)
            {
                OSD::Stop(DrawMainMenu);
                MenuOpened = false;
            }
            //メニューが開いてない
            else if (!MenuOpened)
            {
                OSD::Run(DrawMainMenu);
                MenuOpened = true;
            }
        }

        //チェック中のチートを常に動かす処理
        if (MiniMenuOverview::Cheating)
        {
            for (int i = 0; i < MiniMenuOverview::Entry.size() - 1; i++)
            {
                if (MiniMenuOverview::Entry[i].ProgramRunning)
                {
                    Process::Write32(MiniMenuOverview::Entry[i].Address, MiniMenuOverview::Entry[i].Value);
                }
            }
        }

    }

    //メニューを描写
    bool DrawMainMenu(const Screen &screen)
    {
        /////////////////
        if (screen.IsTop)
            return false;
        /////////////////

        /*
            上画面 800 * 240
            下画面 320 * 240
        */

        static  int     arrow       = 0;
        const   int     entryAmount = MiniMenuOverview::Entry.size() - 1;
        static  bool    scroll_True = entryAmount > 8;
        static  int     scroll      = 0;

        u32 Selecting_Address   =   MiniMenuOverview::Entry[arrow].Address;
        u32 Selecting_Value     =   MiniMenuOverview::Entry[arrow].Value;

        //ベース画面描写
        screen.DrawRect(20, 20, 280, 200, Color::Black);
		screen.DrawRect(22, 22, 276, 196, Color::White, false);

        //タイトル・バージョン表記
        if (!MiniMenuOverview::InformationOpened)
        {
            //(i)を開いてないとき
            screen.DrawSysfont(MiniMenuOverview::TitleName, 37, 25);
            DrawBackword(screen, MiniMenuOverview::VerString, 293, 25, Color::White);
        }

        //タイトル下線部
        const int TitleTextWidth = OSD::GetTextWidth(true, MiniMenuOverview::TitleName);
        screen.DrawRect(37, 42, 32 + TitleTextWidth, 1, Color::White);

        //コピーライトの描写
        const int Copyright_posX = ((320 - OSD::GetTextWidth(false, MiniMenuOverview::Copyright)) / 2);
        screen.Draw(MiniMenuOverview::Copyright, Copyright_posX, 205);

        //矢印
        if (!MiniMenuOverview::InformationOpened)   //(i)を開いてないとき
        {
            const   int     xPos_arrow  = 33;
            const   int     yPos_arrow  = 51;
            if (scroll == 0)
            {
                screen.DrawSysfont(Color::DeepSkyBlue << ">", xPos_arrow, yPos_arrow + 15 * arrow);
            }
            else
            {
                screen.DrawSysfont(Color::DeepSkyBlue << ">", xPos_arrow, yPos_arrow + 15 * 8);
            }
        }

        //チート表示
        const   int     xPos_CheckBox = 48;
        const   int     yPos_CheckBox = 56;
        for(int i = 0; i < MiniMenuOverview::Entry.size() + 1; i++)
        {
            //差分は消す
            if(i + scroll < MiniMenuOverview::Entry.size()){
                //名前の描写
                screen.DrawSysfont(MiniMenuOverview::Entry[i + scroll].name, 60, 53 + (15 * i));

                //Entryの種類の描写
                if (MiniMenuOverview::Entry[i + scroll].type == 0)      //CheckBox
                {
                    screen.DrawRect(xPos_CheckBox, yPos_CheckBox + (15 * i), 9, 9, Color::White);//■
                    if (MiniMenuOverview::Entry[i + scroll].ProgramRunning) 
                    {
                        //有効時の描写
                        screen.DrawPixel(xPos_CheckBox + 7, yPos_CheckBox +  1 + (15 * i), Color::LimeGreen);
                        for (int j = 0; j < 2; j++)
                        {
                            screen.DrawPixel(xPos_CheckBox + 7 - j, yPos_CheckBox +  2 + (15 * i), Color::LimeGreen);
                            screen.DrawPixel(xPos_CheckBox + 6 - j, yPos_CheckBox +  3 + (15 * i), Color::LimeGreen);
                            screen.DrawPixel(xPos_CheckBox + 5 - j, yPos_CheckBox +  4 + (15 * i), Color::LimeGreen);
                            screen.DrawPixel(xPos_CheckBox + 4 - j, yPos_CheckBox +  5 + (15 * i), Color::LimeGreen);
                            screen.DrawPixel(xPos_CheckBox + 3 - j, yPos_CheckBox +  6 + (15 * i), Color::LimeGreen);
                            screen.DrawPixel(xPos_CheckBox + 2 - j, yPos_CheckBox +  5 + (15 * i), Color::LimeGreen);
                        }
                    }
                }
                else if (MiniMenuOverview::Entry[i + scroll].type == 1) //チェック不可
                {
                    screen.DrawRect(xPos_CheckBox + 2, yPos_CheckBox     + (15 * i), 5, 2, Color::White);
                    screen.DrawRect(xPos_CheckBox    , yPos_CheckBox + 2 + (15 * i), 2, 5, Color::White);
                    screen.DrawRect(xPos_CheckBox + 2, yPos_CheckBox + 7 + (15 * i), 5, 2, Color::White);
                    screen.DrawRect(xPos_CheckBox + 7, yPos_CheckBox + 2 + (15 * i), 2, 5, Color::White);
                    screen.DrawRect(xPos_CheckBox + 3, yPos_CheckBox + 3 + (15 * i), 3, 3, Color::White, false);
                }

                //(i)の描写
                if (MiniMenuOverview::Entry[i + scroll].Information)
                {
                    screen.DrawSysfont(Color::DodgerBlue << "(i)", 260, 53 + (15 * i));
                }
            }
            //
            if (i == 9)
                break;
        }
        //(i)の出力
        if (MiniMenuOverview::InformationOpened)
        {
            //基盤
            screen.DrawRect(37, 42, 246, 161, Color::Black);
		    screen.DrawRect(39, 44, 242, 157, Color::White, false);

            //タイトル出力
            screen.DrawSysfont(MiniMenuOverview::Entry[arrow].name, 37, 25);
            
            //説明文表示
            int note_length                 = MiniMenuOverview::Entry[arrow].note.length(); //noteの文字数取得
            const int untilnewLine          = 230;                                          //改行するまでの長さ
            std::string note                = MiniMenuOverview::Entry[arrow].note;
            //説明表示
            int start = 0;
            for (int i = 0; i < note_length; i++)
            {
                int noteTextWidth = OSD::GetTextWidth(true, note.substr(start, i - start));
                if (noteTextWidth > untilnewLine)
                {
                    note.insert(i, 1, '\n');
                    start = i;
                }
                if (note[i] == '\n')
                {
                    start = i;
                }
            }
            DrawSysfont_newLinePlus(screen, note, 43, 45, Color::White);
        }

        //コントロール
        if (!MiniMenuOverview::InformationOpened) //(i)を開いてないとき
        {
            if (Controller::IsKeyPressed(DPadDown))
            {
                if (arrow == entryAmount)
                {
                    arrow   = -1;
                    scroll  = 0;
                }
                else if (arrow >= 8)
                {
                    scroll ++;
                }
                arrow ++;
            }
            if (Controller::IsKeyPressed(DPadUp))
            {
                if (arrow == 0)
                {
                    arrow   = entryAmount;
                    if (scroll_True)
                    {
                        scroll  = entryAmount - 8;
                    }
                }
                else
                {
                    arrow --;
                    if (scroll > 0)
                    {
                        scroll --;
                    }
                }
            }
            if (Controller::IsKeyPressed(A))
            {
                if (MiniMenuOverview::Entry[arrow].type == 0)       //Checkbox
                {
                    //もしoffならon,onならoff
                    if (MiniMenuOverview::Entry[arrow].ProgramRunning)
                    {
                        MiniMenuOverview::Entry[arrow].ProgramRunning = false;
                    }
                    else
                    {
                        MiniMenuOverview::Entry[arrow].ProgramRunning = true;
                    }
                }
                else if(MiniMenuOverview::Entry[arrow].type == 1)   //チェック不可
                {
                    Process::Write32(Selecting_Address, Selecting_Value);
                }
            }
        }
        if (Controller::IsKeyPressed(Y))
        {
            if (MiniMenuOverview::Entry[arrow].Information)
            {
                if (MiniMenuOverview::InformationOpened)
                {
                    //開いてる処理
                    MiniMenuOverview::InformationOpened = false;
                }
                else
                {
                    //閉じてる処理
                    MiniMenuOverview::InformationOpened = true;
                }
            }
        }

        //どれか一つでもチートが有効ならtrueにする
        for (const auto& entry : MiniMenuOverview::Entry)
        {
            if (entry.ProgramRunning)
            {
                MiniMenuOverview::Cheating = true;
                break;
            }
            else
            {
                MiniMenuOverview::Cheating = false;
            }
        }

        //Debug
        /*
        screen.Draw(Utils::Format("entryAmount = %d", entryAmount), 0, 0);
        screen.Draw(Utils::Format("arrow = %d", arrow), 0, 10);
        screen.Draw(Utils::Format("scroll = %d", scroll), 100, 0);
        if (MiniMenuOverview::Cheating)
            screen.Draw("true", 0, 0);
        else
            screen.Draw("false", 0, 0);
        */
        /*
        int testwwwwwwwwww = OSD::GetTextWidth(true, "くけこさしすせそたちつてとなにぬねの");
        screen.Draw(Utils::Format("%d", testwwwwwwwwww), 0, 0);
        //DrawSysfont_newLinePlus(screen, "あ\nい\nう\nえ\nお", 0, 0, Color::Black);
        */

        ////////////
        return true;
    }

}