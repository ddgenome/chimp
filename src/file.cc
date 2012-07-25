// Methods to safely manipulate files.
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cerrno>		// defines errno
#include <cstring>		// for strerror()
#include <iostream>
#include "t_string.h"
#include "file.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// file_name methods
// ctor: provide file path
file_name::file_name(const CH_STD::string& path_)
  throw (bad_file)
  : path(path_), file(), dir(), base(), extension()
{
  // make sure it is not an empty string
  if (path.length() == 0)
    {
      throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":file_name::file_name(): path specified was an empty "
		     "string");
    }
  // split path into directory and file parts
  split_path();
  // split file name into base name and extension
  strip_extension();
}

// dtor: do nothing
file_name::~file_name()
{}

// file_name private methods
// strip extension off the file name (if there is one)
void
file_name::strip_extension()
{
  // find last '.' in the file
  CH_STD::string::size_type pos(file.find_last_of('.'));
  if (pos == file.npos)		// no extension
    {
      base.assign(file);
      extension.erase();
    }
  else
    {
      base.assign(file, 0, pos);
      // put period into extension
      extension.assign(file, pos, file.npos);
    }
  return;
}

// split PATH into directories and file name, assign file and dir
void
file_name::split_path()
{
  // cut off file name
  CH_STD::string::size_type pos(path.find_last_of('/'));
  if (pos == path.npos)
    {
      // no directories in given file name (i.e., it is in cwd)
      file.assign(path);
      // directory is an empty string
      dir.erase();
    }
  else
    {
      // pluck the file name off the end
      file.assign(path, pos + 1, path.npos);
      dir.assign(path, 0, pos);
#if 0				// don't need this
      // erase the file name from path
      path.erase(pos + 1, path.npos);
      // cycle through local copy of path and split it up
      int start_pos(0);
      int end_pos(0);
      while (end_pos != path.npos)
	{
	  end_pos = path.find_first_of('/', start_pos);
	  dirs.push_back(path.substr(pos, end_pos));
	  start_pos = end_pos + 1;
	}
#endif // 0
    }
  return;
}

// file_name public methods
// return the entire path
CH_STD::string
file_name::get_path() const
{
  return path;
}

// return the file portion of the path
CH_STD::string
file_name::get_file() const
{
  return file;
}

// return the directory part
CH_STD::string
file_name::get_directory() const
{
  return dir;
}

// return the base file name
CH_STD::string
file_name::get_base() const
{
  return base;
}

// return the file extension (including period)
CH_STD::string
file_name::get_extension() const
{
  return extension;
}

// file_stat methods
// ctor: set path user provided file path
file_stat::file_stat(const CH_STD::string& path)
  throw (bad_file)
  : name(path), exist(false), regular_file(false), directory_file(false),
    read(false), write(false), no_read(), no_write()
{
  get_stat();
}

// dtor: do nothing
file_stat::~file_stat()
{}

// file_stat private methods
// obtain the file stat information for PATH, return existance of PATH
bool
file_stat::get_stat()
  throw (bad_file)
{
  CH_STD::pair<bool,struct stat> if_stat(get_stat(name.get_path()));
  if (if_stat.first)
    {
      // file exists and stat was successful
      exist = true;
      path_stat = if_stat.second;
      // determine file type and set instance variables
      if (S_ISREG(path_stat.st_mode))
	{
	  regular_file = true;
	}
      else if (S_ISDIR(path_stat.st_mode))
	{
	  directory_file = true;
	}
      // determine file permissions
      if (regular_file || directory_file)
	{
	  is_readable();
	  is_writable();
	}
      else
	{
	  no_read = no_write = "file is not a regular file or directory";
	}
    }
  else				// does not exist
    {
      no_read = no_write = CH_STD::strerror(ENOENT);
    }
  return if_stat.first;
}

// see if PATH is readable, set read, return READ
bool
file_stat::is_readable()
  throw (bad_file)
{
  if (exist)
    {
      // only deal with regular files and directories
      if (regular_file || directory_file)
	{
	  // try to open for reading
	  CH_STD::ifstream test(name.get_path().c_str());
	  if (test)
	    {
	      // opened properly
	      test.close();
	      read = true;
	    }
	  else
	    {
	      // could not open for reading
	      no_read = CH_STD::strerror(errno);
	    }
	}
      else			// not a regular file or directory
	{
	  throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			 ":file_stat::is_readable(): file (" + name.get_path()
			 + ") is not a regular file or directory");
	}
    }
  else
    {
      throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":file_stat::is_readable(): file (" + name.get_path()
		     + ") does not exist");
    }
  return read;
}

// see if path is writable, set write, return WRITE
bool
file_stat::is_writable()
  throw (bad_file)
{
  if (exist)
    {
      // only deal with regular files and directories
      if (regular_file || directory_file)
	{
	  // try to open existing file for writing, do not destroy contents
	  CH_STD::ofstream test(name.get_path().c_str(),
                                CH_STD::ios::in | CH_STD::ios::app);
	  if (test)
	    {
	      // opened properly
	      test.close();
	      write = true;
	    }
	  else
	    {
	      // could not open for reading
	      no_write = CH_STD::strerror(errno);
	    }
	}
      else			// not a regular file or directory
	{
	  throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			 ":file_stat::is_writable(): file (" + name.get_path()
			 + ") is not a regular file or directory");
	}
    }
  else
    {
      throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":file_stat::is_writable(): file (" + name.get_path()
		     + ") does not exist");
    }
  return write;
}

// get the file stat struct for PATH_, return existence,stat
CH_STD::pair<bool,struct stat>
file_stat::get_stat(const CH_STD::string& path)
  throw (bad_file)
{
  struct stat p_stat;
  // ask for the stat and check return value
  if (::stat(path.c_str(), &p_stat) < 0)
    {
      // failure during stat, check errno and throw exception
      int errno_save(errno);	// in case any intermediate fucntions set it
      switch (errno_save)
	{
	case EACCES:		// permission denied
	  throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			 ":file_stat::get_stat(): you cannot access " + path +
			 ": " + CH_STD::strerror(errno_save));
	  break;
	case ENOENT:		// no such file or directory
	  {			// need scope so we can declare variables
	    // create a file_name for the given file
	    file_name p_name(path);
	    // make sure a directory was specified and exists
	    struct stat dir_stat;
	    if (p_name.get_directory().size()
		&& ::stat(p_name.get_directory().c_str(), &dir_stat) < 0)
	      {
		// something is wrong with the directory path
		int errno_dir(errno);
		switch (errno_dir)
		  {
		  case ENOENT:
		    throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__)
				   + ":file_stat::get_stat(): one or more "
				   "directories in " + p_name.get_directory() +
				   " do not exist: " +
				   CH_STD::strerror(errno_dir));
		    break;
		  default:
		    throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__)
				   + ":file_stat::get_stat(): unable to map "
				   "given path (" + p_name.get_directory() +
				   ") to actual directory: " +
				   CH_STD::strerror(errno_dir));
		    break;
		  }
	      }
	    else		// directory exists
	      {
		// directory must also be readable or you would have
		// gotten EACCES when you did the first stat,
		// so it must be that the file does not exist
		return CH_STD::make_pair(false, p_stat);
	      }
	  }
	  break;
	case ENOTDIR:		// component of path is not a directory
	  throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			 ":file_stat::get_stat(): invalid path to file (" +
			 path + "): " + CH_STD::strerror(errno_save));
	  break;
	default:		// OS level error (memory, sym links, etc.)
	  throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			 ":file_stat::get_stat(): unable to map file name (" +
			 path + ") to file: " + CH_STD::strerror(errno_save));
	  break;
	}
    }
  return CH_STD::make_pair(true, p_stat);
}

// file_stat public methods
// return whether file already exists
bool
file_stat::exists() const
{
  return exist;
}

// return whether file is a regular file
bool
file_stat::is_regular() const
{
  return regular_file;
}

// return whether file is a directory
bool
file_stat::is_directory() const
{
  return directory_file;
}

// return whether you have permission to read
bool
file_stat::read_permission() const
{
  return read;
}

// return whether you have permission to write
bool
file_stat::write_permission() const
{
  return write;
}

// return string of why file is unreadable
CH_STD::string
file_stat::why_no_read() const
{
  return no_read;
}

// return string of why you can't write to the file
CH_STD::string
file_stat::why_no_write() const
{
  return no_write;
}

// class to safely (prompt when overwrite) files for output
// ctor: try to open file for output, if it exists, prompt
safe_ofstream::safe_ofstream(const CH_STD::string& path)
  throw (bad_file)
  : CH_STD::ofstream()
{
  open(path);
}

// dtor: do nothing
safe_ofstream::~safe_ofstream()
{}

// safe_open private methods
// open the file, if it is there attempt to overwrite it
void
safe_ofstream::force_open(const CH_STD::string& path)
  throw (bad_file)
{
  // open the file and check the stream
  CH_STD::ofstream::open(path.c_str());
  if (!*this)
    {
      // open was unsuccesful, throw exception
      throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__)
		     + ":safe_ofstream::open(): could not open file "
		     + path + ": " + CH_STD::strerror(errno));
      // NOTE: don't know if errno will work
    }
  return;
}

// safe_open public methods
// open the given file (prompt for overwrite)
void
safe_ofstream::open(const CH_STD::string& path)
  throw (bad_file)
{
  bool is_open(false);
  CH_STD::string file(path);
  while (!is_open)
    {
      // stat file to see if it exists (this will also check permissions)
      file_stat file_stat(file);
      if (file_stat.exists())
	{
	  CH_STD::cerr << PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
	    ":safe_ofstream::safe_ofstream(): file " + file + " exists, "
	    "overwrite? [y/n] ";
	  CH_STD::string answer;
	  CH_STD::cin >> answer;
	  if (answer.at(0) == 'y' || answer.at(0) == 'Y')
	    {
	      // overwrite file
	      force_open(file);
	      is_open = true;
	    }
	  else
	    {
	      CH_STD::cerr << PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		":safe_ofstream::safe_ofstream(): please enter another file "
		"name: (just press enter to cancel) ";
	      CH_STD::string new_file;
	      CH_STD::cin >> new_file;
	      if (new_file.length() == 0)
		{
		  // throw exception
		  throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__)
				 + ":safe_ofstream::safe_ofstream(): did not "
				 "overwrite already existing file " + file);
		}
	      file = new_file;
	    }
	}
      else			// does not exist
	{
	  force_open(file);
	  is_open = true;
	}
    }
  return;
}

CH_END_NAMESPACE

/* $Id: file.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
