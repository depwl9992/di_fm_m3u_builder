//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Main.h"
#include "Precomp.h"
#include <System.IOUtils.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
String iniFileName;
TStringList *channelList;
TStringList *abbrevList;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner) {
	TIniFile *ini;

	if (ParamCount() > 0) {
		iniFileName = ExtractFilePath(Application->ExeName)+ParamStr(1);
	} else {
		iniFileName = ChangeFileExt(Application->ExeName,".ini");
	}

	ini = new TIniFile(iniFileName);

	if (ini) {
		Edit4->Text = ini->ReadString("Defaults","PlayKey","0123456789abcdf012345678");
		RadioGroup1->ItemIndex = ((ini->ReadString("Defaults","URL","Server") == "Server")?1:0);
//		RadioGroup2->ItemIndex = ini->ReadInteger("Defaults","Premium",0);
		RadioGroup2->ItemIndex = 1; // Free no longer available for non-browser stream.
		RadioGroup2->Enabled = false;
	}
	ReadChannelList();
	EnableServerBox(RadioGroup1->ItemIndex == 1);
	//EnablePremium(RadioGroup2->ItemIndex == 1);
	EnablePremium(true);
	//Button4->Enabled = (SaveTextFileDialog1->FileName != "");
}

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action) {
	TIniFile *ini;
	try {
		ini = new TIniFile(iniFileName);

		ini->WriteString("Defaults","PlayKey",Edit4->Text);
		ini->WriteString("Defaults","URL",((RadioGroup1->ItemIndex==1)?"Server":"Playlist"));
		//ini->WriteBool("Defaults","Premium",(bool)(RadioGroup2->ItemIndex));
		ini->WriteBool("Defaults","Premium",true);
	} catch (...) {
	}
}
void __fastcall TForm1::Button1Click(TObject *Sender) {
	if (Edit1->Text == "" || Edit2->Text == "") {
		Status("Title and URL Abbreviation of channel must be included.");
		return;
	}

	int rowCt = StringGrid1->RowCount;
	int rowCur;
	StringGrid1->RowCount++;

	if (StringGrid1->Cells[0][rowCt-1] == "" || StringGrid1->Cells[1][rowCt-1] == "") {
		rowCur = rowCt - 1;
		StringGrid1->RowCount--;
	} else {
		rowCur = rowCt;
	}

	StringGrid1->Cells[0][rowCur] = Edit1->Text;
	StringGrid1->Cells[1][rowCur] = Edit2->Text;
	//Edit1->Text = "";
	//Edit2->Text = "";
	Form1->ActiveControl = Edit1;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift) {
	if (Key == VK_RETURN) {
		Button1->Click();
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SaveClick(TObject *Sender) {
	/** Free:
	 *   Good (96k MP3)
	 *   http://pub#.di.fm:80/di_<channel>?<key>
	 *   http://listen.di.fm/public3/<channel>.pls?listen_key=<key>
	 *   Good (64k AAC)
	 *   http://pub#.di.fm:80/di_<channel>_aac?<key>
	 *   http://listen.di.fm/public1/<channel>.pls?listen_key=<key>
	 *   Low  (40k AAC)
	 *   http://pub#.di.fm:80/di_<channel>_aacplus?<key>
	 *   http://listen.di.fm/public2/<channel>.pls?listen_key=<key>
	 *
	 *  Premium:
	 *   Excellent (128k AAC)
	 *   http://prem#.di.fm:80/<channel>?<key>
	 *   http://listen.di.fm/premium/<channel>.pls?listen_key=<key>
	 *   Excellent (320k MP3)
	 *   http://prem#.di.fm:80/<channel>_hi?<key>
	 *   http://listen.di.fm/premium_high/<channel>.pls?listen_key=<key>
	 *   Good (64k AAC)
	 *   http://prem#.di.fm:80/<channel>_aac?<key>
	 *   http://listen.di.fm/premium_medium/<channel>.pls?listen_key=<key>
	 *   Low (40k AAC)
	 *   http://prem#.di.fm:80/<channel>_aacp?<key>
	 *   http://listen.di.fm/premium_low/<channel>.pls?listen_key=<key>
	 */

	TButton *btn = dynamic_cast<TButton*>(Sender);
	String key = Edit4->Text;
	bool save = false;
	TStringList *m3u = new TStringList();

	if (key == "") {
		Status("Invalid Listen Key");
		return;
	}


	if (true) {
//	if (RadioGroup2->ItemIndex == 1) { // Premium
		String qualityPlsStr[4] = {"premium","premium_high","premium_medium","premium_low"};
		String qualitySrvStr[4] = {"","_hi","_aac","_aacp"};
		String quality[4] = {"128k AAC","320k MP3", "64k AAC", "40k AAC"};
		TCheckBox *qualityCB[4] = {CheckBox1, CheckBox2, CheckBox3, CheckBox4};
		TCheckBox *servers[8] = {CheckBox5, CheckBox6, CheckBox7, CheckBox8, CheckBox9, CheckBox10, CheckBox11, CheckBox12};

		m3u->Clear();
		m3u->Add("#EXTM3U"); // m3u Header

		for (int i = 0; i < StringGrid1->RowCount; i++) {
			String name = StringGrid1->Cells[0][i];
			String abbr = StringGrid1->Cells[1][i];

			// Skip any blank or partially blank entries.

			if (name == "" || abbr == "") {
				continue;
			}

			if (RadioGroup1->ItemIndex == 0) {
				// http://listen.di.fm/<qualPlsStr>/<abbrev>.pls?listen_key=<key>
				// http://listen.di.fm/
				for (int j = 0; j < 4; j++) {
					if (qualityCB[j]->Checked) {
						save = true;
						m3u->Add(Format("#EXTINF:-1,DI.fm (Prem %s) - %s",
							ARRAYOFCONST((quality[j],name))
						));
						m3u->Add(Format("http://listen.di.fm/%s/%s.pls?listen_key=%s",
							ARRAYOFCONST((qualityPlsStr[j],abbr,key))
						));
					}
				}
			} else {
				for (int j = 0; j < 4; j++) {
					if (qualityCB[j]->Checked) {
						for (int k = 0; k < 8; k++) {
							if (servers[k]->Checked) {
								save = true;
								m3u->Add(Format("#EXTINF:-1,DI.fm (Prem %s) - %s",
									ARRAYOFCONST((quality[j],name))
								));
								m3u->Add(Format("http://prem%d.di.fm:80/%s%s?%s",
									ARRAYOFCONST((k+1,abbr,qualitySrvStr[j],key))
								));
							}
						}
					}
				}
			}
		}
	} else {
		String qualityPlsStr[3] = {"public3","public1","public2"};
		String qualitySrvStr[3] = {"","_aac","_aacplus"};
		String quality[3] = {"96k MP3","64k AAC", "40k AAC"};
		TCheckBox *qualityCB[3] = {CheckBox13, CheckBox14, CheckBox15};
		TCheckBox *servers[8] = {CheckBox5, CheckBox6, CheckBox7, CheckBox8, CheckBox9, CheckBox10, CheckBox11, CheckBox12};

		m3u->Clear();
		m3u->Add("#EXTM3U"); // m3u Header
		for (int i = 0; i < StringGrid1->RowCount; i++) {
			String name = StringGrid1->Cells[0][i];
			String abbr = StringGrid1->Cells[1][i];

			// Skip any blank or partially blank entries.
			if (name == "" || abbr == "") continue;

			if (RadioGroup1->ItemIndex == 0) {
				// http://listen.di.fm/<qualPlsStr>/<abbrev>.pls?listen_key=<key>
				// http://listen.di.fm/
				for (int j = 0; j < 3; j++) {
					if (qualityCB[j]->Checked) {
						save = true;
						m3u->Add(Format("#EXTINF:-1,DI.fm (Prem %s) - %s",
							ARRAYOFCONST((quality[j],name))
						));
						m3u->Add(Format("http://listen.di.fm/%s/%s.pls?listen_key=%s",
							ARRAYOFCONST((qualityPlsStr[j],abbr,key))
						));
					}
				}
			} else {
				for (int j = 0; j < 3; j++) {
					if (qualityCB[j]->Checked) {
						for (int k = 0; k < 8; k++) {
							if (servers[k]->Checked) {
								save = true;
								m3u->Add(Format("#EXTINF:-1,DI.fm (Free %s) - %s",
									ARRAYOFCONST((quality[j],name))
								));
								m3u->Add(Format("http://pub%d.di.fm:80/di_%s%s?%s",
									ARRAYOFCONST((k+1,abbr,qualitySrvStr[j],key))
								));
							}
						}
					}
				}
			}
		}
	}
	Memo1->Lines->Assign(m3u);
	bool saveAs = false;
	if (btn == SaveAs) {
		saveAs = true;
	}
	SaveToFile(m3u, save, saveAs);
	//Button4->Enabled = (SaveTextFileDialog1->FileName != "");
}

void TForm1::SaveToFile(TStringList *m3u, bool save,bool saveas) {
	if (save) {
		if (saveas) {
			if (SaveTextFileDialog1->Execute()) {
				m3u->SaveToFile(SaveTextFileDialog1->FileName);
				Status("Saved!");
			} else {
				Status("Playlist not saved.");
			}
		} else {
			if (SaveTextFileDialog1->FileName == "") {
				if (SaveTextFileDialog1->Execute()) {
					m3u->SaveToFile(SaveTextFileDialog1->FileName);
					Status("Saved!");
				} else {
					Status("Playlist not saved.");
				}
			} else {
				m3u->SaveToFile(SaveTextFileDialog1->FileName);
				Status("Saved");
			}
		}
	} else {
		Status("Nothing to save...");
	}
}

void TForm1::Status(String msg) {
	StatusBar1->Panels->Items[0]->Text = (String)msg;
	Timer1->Enabled = true;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Timer1Timer(TObject *Sender) {
	StatusBar1->Panels->Items[0]->Text = "";
	Timer1->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender) {
	for (int i = 0; i < StringGrid1->RowCount; i++) {
		StringGrid1->Rows[i]->Clear();
	}

	StringGrid1->RowCount = 0;

}
//---------------------------------------------------------------------------

void __fastcall TForm1::RadioGroup1Click(TObject *Sender) {
	EnableServerBox(RadioGroup1->ItemIndex == 1);
}
//---------------------------------------------------------------------------

void TForm1::EnableServerBox(bool en) {
	GroupBox1->Enabled = en;
	TCheckBox *cb[8] = {CheckBox5, CheckBox6, CheckBox7, CheckBox8, CheckBox9, CheckBox10, CheckBox11, CheckBox12};
	for (int i = 0; i < 8; i++) {
        cb[i]->Enabled = en;
	}
}
void __fastcall TForm1::RadioGroup2Click(TObject *Sender) {
	EnablePremium(true);
	//EnablePremium(RadioGroup2->ItemIndex == 1);
}
void TForm1::EnablePremium(bool en) {
	GroupBox1->Enabled = en;
	TCheckBox *free[3] = {CheckBox13,CheckBox14,CheckBox15};
	TCheckBox *prem[4] = {CheckBox1, CheckBox2, CheckBox3, CheckBox4};

	for (int i = 0; i < 3; i++) {
		free[i]->Enabled = !en;
		prem[i]->Enabled = en;
		free[i]->Visible = !en;
		prem[i]->Visible = en;
	}

	prem[3]->Enabled = en;
	prem[3]->Visible = en;

}

void __fastcall TForm1::Button4Click(TObject *Sender) {
	bool fnReset = false;
	if (SaveTextFileDialog1->FileName == "") {

		if (StringGrid1->RowCount == 1) {
			if (StringGrid1->Cells[0][0] == "" || StringGrid1->Cells[1][0] == "") {
				// Grid is blank.
				if (Edit1->Text != "" && Edit2->Text != "") {
					// Create playlist if a channel is queued.
					Button1->Click();
				} else {
					MessageDlg("Select a channel or build a playlist first.",mtError,TMsgDlgButtons() << mbOK, 0);
				}
			}
		}

		TPath path;
		SaveTextFileDialog1->FileName = path.GetTempPathW() + "difm.m3u";
		SaveClick(this);
		fnReset = true;
	}
	Status("Playing " + SaveTextFileDialog1->FileName);
	ShellExecute(NULL, L"open", SaveTextFileDialog1->FileName.w_str(),NULL,SaveTextFileDialog1->InitialDir.w_str(),SW_SHOWNORMAL);

	// If temporary playlist created, clear FileName of SaveTextFileDialog1 - we don't want to keep using this file for normal operations.
	if (fnReset) {
        SaveTextFileDialog1->FileName = "";
	}
}

void TForm1::ReadChannelList() {
	channelList = new TStringList();
	abbrevList = new TStringList();
	String filename = ExtractFilePath(Application->ExeName) + "Channels.txt";
	if (FileExists(filename)) {
		TStringList *channels = new TStringList();
		channels->LoadFromFile(filename);
		Memo1->Clear();
		ComboBox1->Clear();
		ComboBox1->Text = "";
		for (int i = 0; i < channels->Count; i++) {
			TStringList *tmp = new TStringList();
			tmp->StrictDelimiter=true;
			tmp->Delimiter = '=';
			tmp->DelimitedText = channels->Strings[i];

			if (tmp->Count == 2) {
				channelList->Add(tmp->Strings[0]);
				abbrevList->Add(tmp->Strings[1]);
			} else {
				Memo1->Lines->Add("!!!Cannot parse " + channels->Strings[i]);
				continue;
			}

			ComboBox1->Items->Add(channelList->Strings[i]);

		}
		ComboBox1->Text = ComboBox1->Items->Strings[0];
		ComboBox1Change(this);
	} else {
        Status("Cannot load file.");
    }
}

void __fastcall TForm1::ComboBox1Change(TObject *Sender) {
	for (int i = 0; i < channelList->Count; i++) {
		if (ComboBox1->Text == channelList->Strings[i]) {
			Edit1->Text = channelList->Strings[i];
			Edit2->Text = abbrevList->Strings[i];
			break;
		}
	}
}
//---------------------------------------------------------------------------

