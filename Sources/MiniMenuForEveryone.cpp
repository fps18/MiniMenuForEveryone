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

    //タッチ
    bool TouchBox(u32 x, u32 y, u32 w, u32 h)
    {
        if (Touch::IsDown())
        {
            UIntVector pos = Touch::GetPosition();
            if (pos.x >= x && pos.y >= y && pos.x <= (x + w) && pos.y <= (y + h))
            {
                return true;
            }
        }
        return false;
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
        int                         type;                       //エントリータイプ
        std::string                 name, note;                 //名前, ノート
        u32                         Address, Value;             //アドレス, 値
        VoidNoArgFunctionPointer    cheat;                      //Func
        //Others
        std::string                 NameBeforeModification;     //名前修正前
        bool                        LongName        = false;    //名前修正の必要があったか
        bool                        Information     = false;    //ノートの有無
        bool                        ProgramRunning  = false;    //プログラムがうごいてるか(type 1のみ)
        bool                        UseFunc         = false;    //関数を使っているか
        bool                        Folder          = false;    //ファイルかどうか
    };

    namespace MiniMenuOverview
    {
        std::string             TitleName, VerString;       //name, version
        std::vector<Cheat_Item> Entry;                      //概要
        static bool Cheating                = false;        //一つでもチートがオンか
        static bool WelcomeMessageOpened    = false;        //ウェルカムメッセージを開いてるか
        static bool InformationOpened       = false;        //(i)を開いてるか
        static bool SettingOpened           = false;        //設定を開いてるか
        static bool Optimize                = false;        //軽量版かどうか
        std::string Copyright               = "Basic developer: FPS 18";
    }

    //type 0:チェックタイプ    1:チェック不可
    void AutoEntry(const std::string& name, short type, u32 Address, u32 Value, const std::string& note, VoidNoArgFunctionPointer cheat = nullptr, bool Folder = false)
    {
        Cheat_Item newItem;
        //重複確認
        bool Duplicate = false;
        for (const Cheat_Item& entry : MiniMenuOverview::Entry)
        {
            if (entry.NameBeforeModification == name && entry.type == type && entry.Address == Address && entry.Value == Value && entry.note == note)
            {
                Duplicate = true;
                break;
            }
        }
        if (!Duplicate)
        {
            //////////////////
            //名前の長さ確認
            const int MaxWidth              = 184;
            newItem.NameBeforeModification  = name;
            int nameTextWidth               = OSD::GetTextWidth(true, newItem.NameBeforeModification);
            newItem.LongName                = nameTextWidth > MaxWidth;
            if (newItem.LongName)
            {
                for (int i = 0; i < newItem.NameBeforeModification.length(); i++)
                {
                    int newnameTextWidth = OSD::GetTextWidth(true, newItem.NameBeforeModification.substr(0, i));
                    if (newnameTextWidth <= MaxWidth)
                    {
                        newItem.name = newItem.NameBeforeModification.substr(0, i);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                newItem.name = newItem.NameBeforeModification;
            }

            //////////////////
            newItem.note = note;
            newItem.type = type;
            newItem.Address = Address;
            newItem.Value = Value;
            //Informationの有無
            if (note != "")
            {
                newItem.Information = true;
            }
            //関数を使っているのか
            if (cheat != nullptr)
            {
                newItem.UseFunc = true;
                newItem.cheat   = cheat;
            }
            //ファイルかどうか
            if (Folder)
            {
                newItem.Folder = true;
            }


            MiniMenuOverview::Entry.push_back(newItem);
        }
    }

    //ファイル
    void new_folder(const std::string &name, const std::string &note)
    {
        AutoEntry(name, 0, 0, 0, note, nullptr, true);
    }

    //ファイル終了
    void point_folder()
    {
        
    }

    //べつの関数利用しない
    void new_entry(const std::string &name, short type, u32 Address, u32 Value, const std::string &note)
    {
        AutoEntry(name, type, Address, Value, note);
    }

    //べつの関数利用する
    void new_entry(const std::string &name, short type, VoidNoArgFunctionPointer cheat, const std::string &note)
    {
        AutoEntry(name, type, 0, 0, note, cheat);
    }


    //const std::string str = title
    //const char ver = version
    //u32 Button = Buttons to display on the display
    void Implementation_MiniMenu (const std::string &str, u32 major, u32 minor, u32 revision, u32 Button, bool Optimize, MenuEntry *entry)
    {    
        MiniMenuOverview::Optimize  = Optimize; //軽量化かどうか
        MiniMenuOverview::TitleName = str;      //タイトルの名前代入
        static bool MenuOpened      = false;    //メニューが開いてるか判別するやつ
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
            for (int i = 0; i < MiniMenuOverview::Entry.size(); i++)
            {
                if (MiniMenuOverview::Entry[i].ProgramRunning)
                {
                    //べつの関数利用する
                    if (MiniMenuOverview::Entry[i].UseFunc)
                    {
                        MiniMenuOverview::Entry[i].cheat();
                    }
                    //べつの関数利用しない
                    else
                    {
                        Process::Write32(MiniMenuOverview::Entry[i].Address, MiniMenuOverview::Entry[i].Value);
                    }
                }
            }
        }

        //チートOFFのときは描写ストップ
        if (entry != nullptr)
        {
            if (!entry->IsActivated())
            {
                OSD::Stop(DrawMainMenu);
                MenuOpened = false;
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
        if (!MiniMenuOverview::InformationOpened && !MiniMenuOverview::WelcomeMessageOpened)
        {
            //(i)とウェルカムメッセージを開いてないとき
            screen.DrawSysfont(MiniMenuOverview::TitleName, 37, 25);
            DrawBackword(screen, MiniMenuOverview::VerString, 293, 25, Color::White);
        }

        //タイトル下線部
        if (!MiniMenuOverview::Optimize)
        {
            //非軽量時
            const int TitleTextWidth = OSD::GetTextWidth(true, MiniMenuOverview::TitleName);
            screen.DrawRect(37, 42, 32 + TitleTextWidth, 1, Color::White);
        }
        else
        {
            //軽量時
            screen.DrawRect(37, 42, 100, 1, Color::White);
        }

        //コピーライトの描写
        const int Copyright_posX = ((320 - OSD::GetTextWidth(false, MiniMenuOverview::Copyright)) / 2);
        screen.Draw(MiniMenuOverview::Copyright, Copyright_posX, 205);

        //矢印
        if (!MiniMenuOverview::InformationOpened && !MiniMenuOverview::SettingOpened && !MiniMenuOverview::WelcomeMessageOpened)  
        { //(i)と設定とウェルカムメッセージを開いてないとき
            const   int     xPos_arrow  = 33;
            const   int     yPos_arrow  = 53;
            if (scroll == 0)
            {
                screen.DrawSysfont(Color::DeepSkyBlue << ">", xPos_arrow, yPos_arrow + 15 * arrow);
            }
            else
            {
                screen.DrawSysfont(Color::DeepSkyBlue << ">", xPos_arrow, yPos_arrow + 15 * 8);
            }
        }

        //チェックボックス,歯車,ファイル,(i)のUI
        auto UI_CheckBox        = [&screen] (u32 x, u32 y)
        {
            screen.DrawRect(x, y, 9, 9, Color::White);
        };
        auto UI_Checkmark       = [&screen] (u32 x, u32 y)
        {
            screen.DrawPixel(x + 7, y +  1, Color::LimeGreen);//Top
            screen.DrawPixel(x + 7, y +  2, Color::LimeGreen);// s
            screen.DrawPixel(x + 6, y +  3, Color::LimeGreen);// s
            screen.DrawPixel(x + 5, y +  4, Color::LimeGreen);// s
            screen.DrawPixel(x + 4, y +  5, Color::LimeGreen);// s
            screen.DrawPixel(x + 3, y +  6, Color::LimeGreen);// s
            screen.DrawPixel(x + 2, y +  5, Color::LimeGreen);// s
            screen.DrawPixel(x + 6, y +  2, Color::LimeGreen);//ss
            screen.DrawPixel(x + 5, y +  3, Color::LimeGreen);//ss
            screen.DrawPixel(x + 4, y +  4, Color::LimeGreen);//ss
            screen.DrawPixel(x + 3, y +  5, Color::LimeGreen);//ss
            screen.DrawPixel(x + 2, y +  6, Color::LimeGreen);//ss
            screen.DrawPixel(x + 1, y +  5, Color::LimeGreen);//ss
        };
        auto UI_Checkmark_light = [&screen] (u32 x, u32 y)
        {
            screen.DrawRect(x + 1, y + 1, 7, 7, Color::LimeGreen);
        };
        auto UI_Uncheck         = [&screen] (u32 x, u32 y)
        {
            screen.DrawRect(x + 2, y     , 5, 2, Color::White);
            screen.DrawRect(x    , y + 2 , 2, 5, Color::White);
            screen.DrawRect(x + 2, y + 7 , 5, 2, Color::White);
            screen.DrawRect(x + 7, y + 2 , 2, 5, Color::White);
            screen.DrawRect(x + 3, y + 3 , 3, 3, Color::White, false);
        };
        auto UI_Uncheck_light   = [&screen] (u32 x, u32 y)
        {
            screen.DrawRect(x, y, 9, 9, Color::Yellow);
        };
        auto UI_Infmark         = [&screen] (u32 x, u32 y)
        {
            screen.DrawRect (x + 1, y    , 7, 9, Color::DodgerBlue);//■
            screen.DrawRect (x    , y + 1, 1, 7, Color::DodgerBlue);//|
            screen.DrawRect (x + 8, y + 1, 1, 7, Color::DodgerBlue);//|
            screen.DrawPixel(x + 4, y + 1,       Color::White);//.
            screen.DrawRect (x + 4, y + 3, 1, 5, Color::White);//|
        };
        auto UI_Infmark_light   = [&screen] (u32 x, u32 y)
        {
            screen.DrawSysfont(Color::DodgerBlue << "(i)", x, y);
        };
        auto UI_Folder          = [&screen] (u32 x, u32 y)
        {
            screen.DrawRect(x, y + 1, 9, 8, Color::Yellow);//■
            screen.DrawRect(x, y + 2, 9, 1, Color::Black);//ー
        };

        //チート表示
        const   int     xPos_CheckBox = 48;
        const   int     yPos_CheckBox = 58;
        const   int     Space         = 15;
        for(int i = 0; i < MiniMenuOverview::Entry.size() + 1; i++)
        {
            //差分は消す
            if(i + scroll < MiniMenuOverview::Entry.size()){
                //名前の描写
                screen.DrawSysfont(MiniMenuOverview::Entry[i + scroll].name, 60, 53 + (Space * i));

                //Entryの種類の描写
                if (MiniMenuOverview::Entry[i + scroll].Folder)  //ファイル
                {
                    UI_Folder(xPos_CheckBox, yPos_CheckBox + (Space * i));
                }
                else if (MiniMenuOverview::Entry[i + scroll].type == 0)      //CheckBox
                {
                    UI_CheckBox(xPos_CheckBox, yPos_CheckBox + (Space * i));
                    if (MiniMenuOverview::Entry[i + scroll].ProgramRunning) 
                    {
                        if (!MiniMenuOverview::Optimize)
                        {
                            //有効時の描写(非軽量化)
                            UI_Checkmark(xPos_CheckBox, yPos_CheckBox + (Space * i));
                        }
                        else
                        {
                            //有効時の描写(軽量版)
                            UI_Checkmark_light(xPos_CheckBox, yPos_CheckBox + (Space * i));
                        }
                    }
                }
                else if (MiniMenuOverview::Entry[i + scroll].type == 1) //チェック不可
                {
                    if (!MiniMenuOverview::Optimize)
                    {
                        //非軽量
                        UI_Uncheck(xPos_CheckBox, yPos_CheckBox + (Space * i));
                    }
                    else
                    {
                        //軽量
                        UI_Uncheck_light(xPos_CheckBox, yPos_CheckBox + (Space * i));
                    }
                }

                //(i)の描写
                if (MiniMenuOverview::Entry[i + scroll].Information)
                {
                    if (!MiniMenuOverview::Optimize)
                    {
                        //非軽量
                        UI_Infmark(260, yPos_CheckBox + (Space * i));
                    }
                    else
                    {
                        //軽量
                        UI_Infmark_light(260, 53 + (Space * i));
                    }
                }
            }
            //
            if (i == 9)
                break;
        }

        //(i)の出力
        static int I_text = 0;
        if (MiniMenuOverview::InformationOpened)
        {
            //基盤
            screen.DrawRect(37, 42, 246, 161, Color::Black);
		    screen.DrawRect(39, 44, 242, 157, Color::White, false);

            //タイトル出力
            screen.DrawSysfont(MiniMenuOverview::Entry[I_text].name, 37, 25);
            
            //説明文表示
            int note_length                 = MiniMenuOverview::Entry[I_text].note.length(); //noteの文字数取得
            const int untilnewLine          = 226;                                          //改行するまでの長さ
            std::string note                = MiniMenuOverview::Entry[I_text].note;
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
        //設定画面
        if (MiniMenuOverview::SettingOpened)
        {
            //基盤
            screen.DrawRect(37, 42, 246, 161, Color::Black);
		    screen.DrawRect(39, 44, 242, 157, Color::White, false);
            //タイトル
            screen.DrawRect(144, 52, 33, 20, Color::DarkGrey);
		    screen.DrawSysfont("設定", 147, 53, Color::Black);
        }

        //コントロール
        if (!MiniMenuOverview::WelcomeMessageOpened)    //ウェルカムメッセージが開いてないとき
        {
            if (!MiniMenuOverview::InformationOpened && !MiniMenuOverview::SettingOpened) //(i)と設定を開いてないとき
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
                if (Controller::IsKeyPressed(DPadLeft))
                {
                    //一番上
                    if (arrow == 0)
                    {
                        arrow   = entryAmount;
                        if (scroll_True)
                        {
                            scroll  = entryAmount - 8;
                        }
                    }
                    //上から4番以内
                    else if (arrow - 4 < 0)
                    {
                        arrow   = 0;
                    }
                    else
                    {
                        arrow = arrow - 4;
                        if (scroll > 0)
                        {
                            if (scroll - 4 < 0)
                            {
                                scroll = 0;
                            }
                            else
                            {
                                scroll = scroll - 4;
                            }
                        }
                    }
                }
                if (Controller::IsKeyPressed(DPadRight))
                {
                    //一番下
                    if (arrow == entryAmount)
                    {
                        arrow = 0;
                        scroll = 0;
                    }
                    //下から4つ以内
                    else if (arrow > entryAmount - 4)
                    {
                        scroll += entryAmount - arrow;
                        arrow = entryAmount;
                    }
                    else
                    {
                        if (scroll_True && arrow > 4)
                        {
                            if (arrow - 4 < 4)
                            {
                                scroll = arrow - 4;
                            }
                            else
                            {
                                scroll = scroll + 4;
                            }
                        }
                        arrow = arrow + 4;
                    }
                }
                if (Controller::IsKeyPressed(A))
                {
                    if (!MiniMenuOverview::Entry[arrow].Folder)
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
                            //べつの関数利用する
                            if(MiniMenuOverview::Entry[arrow].UseFunc)
                            {
                                MiniMenuOverview::Entry[arrow].cheat();
                            }
                            //べつの関数利用しない
                            else
                            {
                                Process::Write32(Selecting_Address, Selecting_Value);
                            }
                        }
                    }
                    else    //ファイルのときの処理
                    {

                    }
                }
                //タッチ処理
                if (Controller::IsKeyPressed(Touchpad))
                {
                    //Box
                    for (int i = 0; i < MiniMenuOverview::Entry.size() + 1; i++)
                    {
                        if (TouchBox(xPos_CheckBox, yPos_CheckBox + (Space * i), 9, 9))
                        {
                            //チェックボックス
                            if (MiniMenuOverview::Entry[i + scroll].type == 0)
                            {
                                if (MiniMenuOverview::Entry[i + scroll].ProgramRunning)
                                {
                                    MiniMenuOverview::Entry[i + scroll].ProgramRunning = false;
                                }
                                else
                                {
                                    MiniMenuOverview::Entry[i + scroll].ProgramRunning = true;
                                }
                            }
                        }
                    }
                    //(i)
                    for (int i = 0; i < MiniMenuOverview::Entry.size() + 1; i++)
                    {
                        if (TouchBox(1 + 260, yPos_CheckBox + (Space * i), 7, 9))
                        {
                            if (MiniMenuOverview::Entry[i + scroll].Information)
                            {
                                I_text = i + scroll;
                                MiniMenuOverview::InformationOpened = true;
                            }
                        }
                    }
                }
            }
            if (!MiniMenuOverview::SettingOpened)//設定を開いてないとき
            {
                if (Controller::IsKeyPressed(Y))
                {
                    //閉じてるときの処理
                    if (MiniMenuOverview::Entry[arrow].Information && !MiniMenuOverview::InformationOpened)
                    {
                        I_text = arrow;
                        MiniMenuOverview::InformationOpened = true;
                    }
                    //開いてるときの処理
                    else if (MiniMenuOverview::InformationOpened)
                    {
                        MiniMenuOverview::InformationOpened = false;
                    }
                }
            }
            if (!MiniMenuOverview::InformationOpened)//(i)を開いてないとき
            {
                if (Controller::IsKeyPressed(X))
                {
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

        //起動時処理(ウェルカムメッセージ)
        if (!Directory::IsExists("MiniMenu"))
        {
            //基盤
            screen.DrawRect(37, 42, 246, 161, Color::Black);
		    screen.DrawRect(39, 44, 242, 157, Color::White, false);

            screen.DrawSysfont("使い方", 140.5, 25);

            screen.DrawSysfont("A:決定,ON/OFF", 60, 50);
            screen.DrawSysfont("↑/↓: 選択を変える", 60, 70);
            screen.DrawSysfont("X:設定  Y:説明", 60, 90);

            //UIの説明
            screen.DrawSysfont(Color::DeepSkyBlue << ">", 60, 110);
            screen.DrawSysfont("カーソル", 100, 110);

            UI_CheckBox(60, 135);
            UI_CheckBox(75, 135);
            UI_Checkmark(75, 135);
            screen.DrawSysfont("チェックボックス", 100, 130);

            UI_Uncheck(60, 155);
            screen.DrawSysfont("一度のみ実行", 100, 150);

            screen.DrawRect(113, 172, 94, 20, Color::DarkGrey);
            screen.DrawRect(113, 172, 94, 20, Color::White, false);
            screen.DrawSysfont("Aを押して続行", 116, 173, Color::Black);

            if (Controller::IsKeyPressed(A) || TouchBox(113, 172, 94, 20))
            {
                Directory::Create("MiniMenu");
            }

            MiniMenuOverview::WelcomeMessageOpened = true;
        }
        else
        {
            MiniMenuOverview::WelcomeMessageOpened = false;
        }
        return true;
    }

}