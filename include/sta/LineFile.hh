// OpenSTA, Static Timing Analyzer
// Copyright (c) 2024, Silimate, Inc.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "StringUtil.hh"

namespace sta {

class LineFile
{
public:
  LineFile();
  LineFile(string filename);
  LineFile(string filename, int startline, int endline,
           int startpos, int endpos);
  ~LineFile() { }
  const string filename() { return filename_; }
  const int startline() { return startline_; }
  const int endline() { return endline_; }
  const int startpos() { return startpos_; }
  const int endpos() { return endpos_; }
  const char *c_str();
  const string str();
  void incrFromText(string text); // TODO

private:
  string filename_;
  int startline_, endline_, startpos_, endpos_;
};

LineFile::LineFile() :
  filename_(""),
  startline_(1),
  endline_(1),
  startpos_(1),
  endpos_(1)
{
}

LineFile::LineFile(string filename) :
  filename_(filename),
  startline_(1),
  endline_(1),
  startpos_(1),
  endpos_(1)
{

}

LineFile::LineFile(string filename,
                   int startline,
		   int endline,
		   int startpos,
		   int endpos) :
  filename_(filename),
  startline_(startline),
  endline_(endline),
  startpos_(startpos),
  endpos_(endpos)
{

}

const char *
LineFile::c_str()
{
  return stringPrint("%s:%d.%d-%d.%d", filename_.c_str(),
		     startline_, startpos_,
		     endline_, endpos_);
}

const string
LineFile::str()
{
  return stdstrPrint("%s:%d.%d-%d.%d", filename_.c_str(),
		     startline_, startpos_,
		     endline_, endpos_);
}

void
LineFile::incrFromText(string text)
{
  int len = text.length();
  for (int i = 0; i < len; i++) {
    if (text[i] == '\n') {
      endline_++;
      endpos_ = 1;
    } else {
      endpos_++;
    }
  }
}

} // namespace
