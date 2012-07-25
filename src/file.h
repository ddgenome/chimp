// -*- C++ -*-
// Classes to safely manipulate files.
// Copyright (C) 2004 David Dooling <banjo@users.sourceforge.net>
//
// This file is part of CHIMP.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
#ifndef CH_FILE_H
#define CH_FILE_H 1

#include <fstream>		// to check permissions
#include <string>
#include <utility>		// pair<>
#include <unistd.h>		// POSIX constants and functions
#include <sys/types.h>		// primative types
#include <sys/stat.h>		// stat()
#include "except.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// file handling utilities
class file_name
{
  CH_STD::string path;		// entire path and file name
  CH_STD::string file;		// entire file name
  CH_STD::string dir;		// directory portion of path
  CH_STD::string base;		// file name without extension
  CH_STD::string extension;	// any extension on file

private:
  // prevent copy construction and assignment
  file_name(const file_name&);
  file_name& operator=(const file_name&);
  // split PATH into directories and file name, assign file and dir
  void split_path();
  // strip extension off the file name (if there is one)
  void strip_extension();
public:
  // ctor: must provide file name
  explicit file_name(const CH_STD::string& path_)
    throw (bad_file); // this
  // dtor: do nothing
  ~file_name();

  // return the entire path
  CH_STD::string get_path() const;
  // return the file portion of the path
  CH_STD::string get_file() const;
  // return the directory part
  CH_STD::string get_directory() const;
  // return the stripped file name
  CH_STD::string get_base() const;
  // return the file extension (including period)
  CH_STD::string get_extension() const;
}; // end class file_name

// class to find out stuff about a file
class file_stat
{
  file_name name;		// file name information
  struct stat path_stat;	// file stat information
  bool exist;			// does it exist
  bool regular_file;		// is it a regular file
  bool directory_file;		// is it a directory
  bool read;			// can you read it
  bool write;			// can you write it
  CH_STD::string no_read;	// why you can't read
  CH_STD::string no_write;	// why you can't write

  typedef CH_STD::pair<CH_STD::string,CH_STD::string> dir_file_pair;

private:
  // prevent copy construction and assignment
  file_stat(const file_stat&);
  file_stat& operator=(const file_stat&);
  // obtain the file stat information for PATH, return existance of PATH
  bool get_stat()
    throw (bad_file); // get_stat(), is_readable(), is_writable()
  // return whether PATH is readable, set read, return READ
  bool is_readable()
    throw (bad_file); // this
  // return whether PATH is writable, set write, return WRITE
  bool is_writable()
    throw (bad_file); // this
  // obtain the file stat struct for PATH_, return <true,stat> if ok
  // return false if file does not exist (and there are no other problems)
  static CH_STD::pair<bool,struct stat> get_stat(const CH_STD::string& path)
    throw (bad_file); // this
public:
  // ctor: set path to user provided path
  explicit file_stat(const CH_STD::string& path_)
    throw (bad_file); // file_name(), get_stat()
  // dtor: do nothing
  ~file_stat();

  // return whether a file exists
  bool exists() const;
  // return whether file is a regular file
  bool is_regular() const;
  // return whether file is a directory
  bool is_directory() const;
  // return whether you have permissions to read
  bool read_permission() const;
  // return string of why file is unreadable
  CH_STD::string why_no_read() const;
  // return whether you have permission to write
  bool write_permission() const;
  // return string of why you can't write to the file
  CH_STD::string why_no_write() const;
}; // end class file_stat

// class to safely open (prompt for overwrite) files for output
class safe_ofstream : public CH_STD::ofstream
{
private:
  // prevent copy construction and assignment
  safe_ofstream(const safe_ofstream&);
  safe_ofstream& operator=(const safe_ofstream&);
  // open the file whether it is there or not
  void force_open(const CH_STD::string& path)
    throw (bad_file); // this
public:
  // ctor: (default) don't do anything yet
  safe_ofstream();
  // ctor: try to open file
  explicit safe_ofstream(const CH_STD::string& path)
    throw (bad_file); // open()
  // dtor: parent will close stream if still open
  ~safe_ofstream();

  // open the file (prompting for overwrite)
  void open(const CH_STD::string& path)
    throw (bad_file); // this, force_open()
}; // end class safe_file

CH_END_NAMESPACE

#endif // not CH_FILE_H

/* $Id: file.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
