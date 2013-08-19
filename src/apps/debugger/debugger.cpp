/*
** Copyright (C) 1998-2013 George Tzanetakis <gtzan@cs.uvic.ca>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <marsyas/CommandLineOptions.h>
#include <marsyas/core/MarSystemManager.h>
#include <marsyas/debug/recorder.h>
#include <marsyas/debug/file_io.h>
#include <marsyas/debug/debugger.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>

using namespace std;
using namespace Marsyas;

int perform_record( MarSystem * system, const std::string & filename, int tick_count );
int perform_compare( MarSystem * system, const std::string & filename );

void report
( const Debug::Record & record, const Debug::BugReport & bugs, int tick_count );
void report_bugs
( const Debug::Record & record, const Debug::BugReport & bugs, int tick_count );
void print_header(int tick_count);
void print_entry
(const std::string & path, const Debug::Record::Entry & record, const Debug::Bug & bug);

void print_usage()
{
  cout << "Usage: marsyas-debug <system-file> [-r <recording-file>] [-c <reference-file>] [-t <tick-count>] [-b]" << endl;
}

void print_help()
{
  static const unsigned int opt_width = 20;

  cout << endl;
  cout << "'marsyas-debug' loads a marsystem from file, runs it and prints debugging information." << endl;
  cout << endl;

  print_usage();
  cout << endl;

  cout << "Options:" << endl;
  cout << left
       << setw(opt_width) << "-u  --usage"
       << " : display short usage info" << endl
       << setw(opt_width) << "-h  --help"
       << " : display this information" << endl
       << setw(opt_width) << "-r --record <file>"
       << " : record to <file>" << endl
       << setw(opt_width) << "-c --compare <file>"
       << " : compare to recording in <file>" << endl
       << setw(opt_width) << "-t --ticks <number>"
       << " : perform <number> ticks" << endl
       << setw(opt_width) << "-b --bugs-only"
       << " : only report bugs" << endl;
}

int main (int argc, const char *argv[])
{
  CommandLineOptions opt;
  opt.addStringOption("record", "r", "");
  opt.addStringOption("compare", "c", "");
  opt.addNaturalOption("ticks", "t", -1);
  opt.addBoolOption("bugs-only", "b", false);
  opt.addBoolOption("help", "h", false);
  opt.addBoolOption("usage", "u", false);
  opt.readOptions(argc, argv);

  if (opt.getBoolOption("help"))
  {
    print_help();
    return 0;
  }

  if (opt.getBoolOption("usage"))
  {
    print_usage();
    return 0;
  }

  const std::vector<std::string> & arguments = opt.getRemaining();
  if (!arguments.size())
  {
    cout << "Missing system file." << endl;
    print_usage();
    return 1;
  }

  std::string system_filename = arguments[0];
  std::string record_filename = opt.getStringOption("record");
  std::string compare_filename = opt.getStringOption("compare");

  cout << "Using system file: " << system_filename << endl;
  if (!record_filename.empty())
    cout << "Recording to file: " << record_filename << endl;
  if (!compare_filename.empty())
    cout << "Comparing with file: " << compare_filename << endl;

  MarSystemManager mng;
  ifstream system_file_stream( system_filename );
  MarSystem *system = mng.getMarSystem(system_file_stream);
  if (!system) {
    cout << "Failed to load plugin file." << endl;
    return 1;
  }

  Debug::Recorder recorder(system);
  Debug::FileReader *reader = 0;
  Debug::FileWriter *writer = 0;

  if (!compare_filename.empty())
    reader = new Debug::FileReader(compare_filename);
  if (!record_filename.empty())
    writer = new Debug::FileWriter(record_filename, system);

  int ticks_remaining = (int) opt.getNaturalOption("ticks");
  if (ticks_remaining < 1)
  {
    if (reader)
      ticks_remaining = -1;
    else
      ticks_remaining = 1;
  }

  bool bugs_only = opt.getBoolOption("bugs-only");

  int result = 0;
  int tick_count = 0;

  while(ticks_remaining)
  {
    Debug::Record reference_record;
    bool have_reference = reader && reader->read(reference_record);
    if (!have_reference && ticks_remaining < 0)
      break;

    system->tick();
    ++tick_count;

    recorder.commit();
    const Debug::Record & actual_record = recorder.record();

    Debug::BugReport bugs;
    if (have_reference)
      Debug::compare(actual_record, reference_record, bugs);

    if (!bugs.empty())
      result = 1;

    if (bugs_only)
      report_bugs(actual_record, bugs, tick_count);
    else
      report(actual_record, bugs, tick_count);

    if (writer)
      writer->write( actual_record );

    recorder.clear();

    if (ticks_remaining > 0)
      --ticks_remaining;
  }

  return result;
}

void report( const Debug::Record & record,
             const Debug::BugReport & bugs,
             int tick_count )
{
  print_header(tick_count);

  for (const auto & record_entry : record.entries())
  {
    const std::string & path = record_entry.first;
    const Debug::Record::Entry & entry = record_entry.second;
    Debug::Bug bug;

    try
    {
      bug = bugs.at(path);
    }
    catch (const std::out_of_range & e)
    {}

    print_entry(path, entry, bug);
  }
}

void report_bugs( const Debug::Record & record,
                  const Debug::BugReport & bugs,
                  int tick_count )
{
  if (bugs.empty())
    return;

  print_header(tick_count);

  for (const auto & bug_entry : bugs)
  {
    const std::string & path = bug_entry.first;
    const Debug::Bug & bug = bug_entry.second;
    Debug::Record::Entry record_entry;

    try
    {
      record_entry = record.entries().at(path);
    }
    catch (const std::out_of_range & e)
    {}

    print_entry(path, record_entry, bug);
  }
}

static const unsigned int field_width = 13;
static const unsigned int tick_field_width = 6;

void print_header(int tick_count)
{
  std::ostringstream tick_string;
  tick_string << "# " << tick_count;

  std::string head_separator((tick_field_width+3)
                             + (field_width+3)*3
                             + field_width,
                             '-');

  cout << head_separator << endl;

  cout << std::left
       << setw(tick_field_width) << tick_string.str() << " | "
       << setw(field_width) << "Real Time" << " | "
       << setw(field_width) << "CPU Time" << " | "
       << setw(field_width) << "Max Dev" << " | "
       << setw(field_width) << "Avg Dev"
       << endl;

  cout << head_separator << endl;
}

void print_entry(const std::string & path,
                 const Debug::Record::Entry & record,
                 const Debug::Bug & bug)
{
  cout << path << endl;
  cout << std::left
       << setw(tick_field_width) << "" << " | "
       << setw(field_width) << record.real_time << " | "
       << setw(field_width) << record.cpu_time << " | "
       << setw(field_width) << bug.max_deviation << " | "
       << setw(field_width) << bug.average_deviation
       << endl;
}
