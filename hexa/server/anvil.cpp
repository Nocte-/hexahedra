//---------------------------------------------------------------------------
// server/anvil_generator.cpp
//
// This file is part of Hexahedra.
//
// Hexahedra is free software; you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "anvil.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <map>
#include <unordered_map>
#include <zlib.h>

#include <boost/array.hpp>
#include <boost/shared_array.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <hexa/block_types.hpp>

using namespace boost::property_tree;
namespace fs = boost::filesystem;

namespace hexa {

class bad_region : public std::exception
{
private:
  const fs::path path;
  const std::string message;
public:
  bad_region(const fs::path path, const std::string& m)
    : path(path), message(m + " (" + path.string() + ")")
  {
  }

  ~bad_region() throw() { }

  const char* what() const throw() {
    return message.c_str();
  }

  const fs::path where() {
    return path;
  }
};

namespace {


typedef uint8_t uint8;
typedef int8_t sint8;
typedef uint16_t uint16;
typedef int16_t sint16;
typedef uint32_t uint32;
typedef int32_t sint32;
typedef uint64_t uint64;
typedef int64_t sint64;

#ifdef WIN32
#  include <winsock2.h>
#else
#  include <arpa/inet.h>
#endif

inline uint64 ntohll(uint64 v)
{
  if(htons(1) == 1) // check if already big-endian
        return v;
  return (uint64)ntohl(v & 0x00000000ffffffff) << 32 | (uint64)ntohl( (v >> 32) & 0x00000000ffffffff);
}

void my_itoa(int value, std::string &buf, int base)
{
  std::string hexarray("0123456789abcdefghijklmnopqrstuvwxyz");
  int i = 30;
  buf = "";

  if(!value)
    buf = "0";

  for(; value && i; --i, value /= base)
  {
    buf.insert(buf.begin(), (char)hexarray[value % base]);
  }
}

void putSint64(uint8 *buf, sint64 value)
{
  uint64 nval = ntohll(value);
  memcpy(buf, &nval, 8);
}

void putSint32(uint8 *buf, sint32 value)
{
  uint32 nval = htonl(value);
  memcpy(buf, &nval, 4);
}

void putSint16(uint8 *buf, short value)
{
  short value2=htons(value);
  memcpy(buf, &value2, 2);
}

void putFloat(uint8 *buf, float value)
{
  uint32 nval;
  memcpy(&nval, &value, 4);
  nval = htonl(nval);
  memcpy(buf, &nval, 4);
}

void putDouble(uint8 *buf, double value)
{
  uint64 nval;
  memcpy(&nval, &value, 8);
  nval = ntohll(nval);
  memcpy(buf, &nval, 8);
}

double getDouble(uint8 *buf)
{
  double val;
  uint64 ival = *reinterpret_cast<const sint64*>(buf);
  ival = ntohll(ival);
  memcpy(&val, &ival, 8);
  return val;
}

float getFloat(uint8 *buf)
{
  float val;
  int ival = ntohl(*reinterpret_cast<const sint32*>(buf));
  memcpy(&val, &ival, 4);
  return val;
}

sint64 getSint64(uint8 *buf)
{
  sint64 val;
	val = *reinterpret_cast<const sint16*>(buf);
	val = ntohll(val);
  return val;
}

sint32 getSint32(uint8 *buf)
{
  int val = ntohl(*reinterpret_cast<const sint32*>(buf));
  return val;
}

sint32 getSint16(uint8 *buf)
{
  short val = ntohs(*reinterpret_cast<const sint16*>(buf));

  return val;
}

std::string base36_encode(int value)
{
  std::string output;
  my_itoa((int)abs(value), output, 36);
  if(value < 0)
    output.insert (output.begin(), '-');

  return output;
}

enum
{
  TAG_END        = 0,
  TAG_BYTE       = 1,
  TAG_SHORT      = 2,
  TAG_INT        = 3,
  TAG_LONG       = 4,
  TAG_FLOAT      = 5,
  TAG_DOUBLE     = 6,
  TAG_BYTE_ARRAY = 7,
  TAG_STRING     = 8,
  TAG_LIST       = 9,
  TAG_COMPOUND   = 10
};

struct NBT_value
{
  std::uint8_t type;
  std::string name;
  void *value;
};

struct NBT_list
{
  std::string name;
  char tagId;
  int length;
  void **items;
};

struct NBT_byte_array
{
  std::string name;
  int length;
  std::uint8_t *data;
};

struct NBT_struct
{
  std::uint8_t *blocks;
  std::uint8_t *data;
  std::uint8_t *blocklight;
  std::uint8_t *skylight;
  std::int16_t x;
  std::int16_t z;

  std::string name;
  std::vector<NBT_value> values;
  std::vector<NBT_list> lists;
  std::vector<NBT_byte_array> byte_arrays;
  std::vector<NBT_struct> compounds;
};

//Data reading
int TAG_Byte(std::uint8_t *input, char *output);
int TAG_Short(std::uint8_t *input, std::int16_t *output);
int TAG_Int(std::uint8_t *input, int *output);
int TAG_Long(std::uint8_t *input, std::int64_t *output);
int TAG_Float(std::uint8_t *input, float *output);
int TAG_Double(std::uint8_t *input, double *output);
int TAG_String(std::uint8_t *input, std::string *output);

int TAG_Byte_Array(std::uint8_t *input, NBT_byte_array *output);
int TAG_List(std::uint8_t *input, NBT_list *output);
int TAG_Compound(std::uint8_t *input, NBT_struct *output, bool start = false);

//Get data from struct
std::uint8_t *get_NBT_pointer(NBT_struct *input, std::string TAG);
//template <typename customType>
//inline bool get_NBT_value(NBT_struct *input, std::string TAG, customType *value);
template <typename customType>
bool get_NBT_value(NBT_struct *input, std::string TAG, customType *value)
{
  for(unsigned i = 0; i < input->values.size(); i++)
  {
    if(input->values[i].name == TAG)
    {
      *value = *(customType *)input->values[i].value;
      return true;
    }
  }

  for(unsigned j = 0; j < input->compounds.size(); j++)
  {
    return get_NBT_value(&input->compounds[j], TAG, value);
  }
  return false;
}

NBT_list *get_NBT_list(NBT_struct *input, std::string TAG);

int dumpNBT_string(std::uint8_t *buffer, std::string name);
int dumpNBT_value(NBT_value *input, std::uint8_t *buffer);
int dumpNBT_struct(NBT_struct *input, std::uint8_t *buffer, bool list = false);
int dumpNBT_byte_array(NBT_byte_array *input, std::uint8_t *buffer, bool list = false);
int dumpNBT_list(NBT_list *input, std::uint8_t *buffer);

bool freeNBT_struct(NBT_struct *input);
bool freeNBT_list(NBT_list *input);

int TAG_Byte(std::uint8_t *input, char *output)
{
  *output = input[0];
  return 1;
}

int TAG_Short(std::uint8_t *input, std::int16_t *output)
{
  *output = getSint16(input);
  return 2;
}

int TAG_Int(std::uint8_t *input, int *output)
{
  *output = getSint32(input);
  return 4;
}

int TAG_Long(std::uint8_t *input, std::int64_t *output)
{
  *output = getSint64(input);
  return 8;
}

int TAG_Float(std::uint8_t *input, float *output)
{
  *output = getFloat(input);
  return 4;
}

int TAG_Double(std::uint8_t *input, double *output)
{
  *output = getDouble(input);
  return 8;
}

int TAG_String(std::uint8_t *input, std::string *output)
{
  int strLen = getSint16(&input[0]);
  *output = "";

  for(int i = 0; i < strLen; i++)
  {
    *output += input[i+2];
  }
  return strLen+2;
}

int TAG_Byte_Array(std::uint8_t *input, NBT_byte_array *output)
{
  int curpos = 0;
  curpos      += TAG_Int(&input[curpos], &output->length);
  output->data = new std::uint8_t[output->length];
  memcpy(output->data, &input[curpos], output->length);
  curpos      += output->length;
  return curpos;
}

int TAG_List(std::uint8_t *input, NBT_list *output)
{
  int curpos = 0;
  curpos += TAG_Byte(&input[curpos], &output->tagId);
  curpos += TAG_Int(&input[curpos], &output->length);

  //If zero length list
  if(!output->length)
    return curpos;


  switch(output->tagId)
  {
  case TAG_BYTE:
    output->items = (void **)new char *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new char;
      curpos          += TAG_Byte(&input[curpos], (char *)output->items[i]);
    }
    break;

  case TAG_SHORT:
    output->items = (void **)new std::int16_t *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new std::int16_t;
      curpos          += TAG_Short(&input[curpos], (std::int16_t *)output->items[i]);
    }
    break;

  case TAG_INT:
    output->items = (void **)new int *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new int;
      curpos          += TAG_Int(&input[curpos], (int *)output->items[i]);
    }
    break;

  case TAG_LONG:
    output->items = (void **)new std::int64_t *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new std::int64_t;
      curpos          += TAG_Long(&input[curpos], (std::int64_t *)output->items[i]);
    }
    break;

  case TAG_FLOAT:
    output->items = (void **)new float *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new float;
      curpos          += TAG_Float(&input[curpos], (float *)output->items[i]);
    }
    break;

  case TAG_DOUBLE:
    output->items = (void **)new double *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new double;
      curpos          += TAG_Double(&input[curpos], (double *)output->items[i]);
    }
    break;

  case TAG_BYTE_ARRAY:
    output->items = (void **)new NBT_byte_array *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new NBT_byte_array;
      curpos          += TAG_Byte_Array(&input[curpos], (NBT_byte_array *)output->items[i]);
    }
    break;

  case TAG_STRING:
    output->items = (void **)new std::string *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new std::string;
      curpos          += TAG_String(&input[curpos], (std::string *)output->items[i]);
    }
    break;

  case TAG_LIST:
    output->items = (void **)new NBT_list *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new NBT_list;
      curpos          += TAG_List(&input[curpos], (NBT_list *)output->items[i]);
    }
    break;

  case TAG_COMPOUND:
    output->items = (void **)new NBT_struct *[output->length];
    for(int i = 0; i < output->length; i++)
    {
      output->items[i] = (void *)new NBT_struct;
      curpos          += TAG_Compound(&input[curpos], (NBT_struct *)output->items[i]);
    }
    break;
  }

  return curpos;
}


int TAG_Compound(std::uint8_t *input, NBT_struct *output, bool start)
{
  char tagType = 1;
  std::string name;
  int curpos   = 0;

  while(tagType != 0)
  {
    curpos += TAG_Byte(&input[curpos], &tagType);
    if(tagType == 0)
    {
      return curpos;
    }
    curpos += TAG_String(&input[curpos], &name);
    NBT_value value;
    value.type     = tagType;
    value.name     = name;
    NBT_byte_array bytearray;
    bytearray.name = name;
    NBT_list list;
    list.name      = name;
    NBT_struct compound;
    compound.name  = name;

    switch(tagType)
    {
    case TAG_BYTE:
      value.value = (void *)new char;
      curpos     += TAG_Byte(&input[curpos], (char *)value.value);
      output->values.push_back(value);
      break;

    case TAG_SHORT:
      value.value = (void *)new std::int16_t;
      curpos     += TAG_Short(&input[curpos], (std::int16_t *)value.value);
      output->values.push_back(value);
      break;

    case TAG_INT:
      value.value = (void *)new int;
      curpos     += TAG_Int(&input[curpos], (int *)value.value);
      output->values.push_back(value);
      break;

    case TAG_LONG:
      value.value = (void *)new std::int64_t;
      curpos     += TAG_Long(&input[curpos], (std::int64_t *)value.value);
      output->values.push_back(value);
      break;

    case TAG_FLOAT:
      value.value = (void *)new float;
      curpos     += TAG_Float(&input[curpos], (float *)value.value);
      output->values.push_back(value);
      break;

    case TAG_DOUBLE:
      value.value = (void *)new double;
      curpos     += TAG_Double(&input[curpos], (double *)value.value);
      output->values.push_back(value);
      break;

    case TAG_BYTE_ARRAY:
      curpos += TAG_Byte_Array(&input[curpos], &bytearray);
      //Special handling with lightmaps
      if(bytearray.length == 0)
      {
        //If zero sized lightmap, generate a new empty lightmap
        if(bytearray.name == "BlockLight" || bytearray.name == "SkyLight")
        {
          bytearray.length = 16*16*16/2;
          bytearray.data   = new std::uint8_t[bytearray.length];
          memset(bytearray.data, 0, bytearray.length);
        }
      }
      output->byte_arrays.push_back(bytearray);
      break;

    case TAG_STRING:
      value.value = (void *)new std::string;
      curpos     += TAG_String(&input[curpos], (std::string *)value.value);
      output->values.push_back(value);
      break;

    case TAG_LIST:
      curpos += TAG_List(&input[curpos], &list);
      output->lists.push_back(list);
      break;

    case TAG_COMPOUND:
      if(start)
        curpos += TAG_Compound(&input[curpos], output);
      else
      {
        curpos += TAG_Compound(&input[curpos], (NBT_struct *)&compound);
        output->compounds.push_back(compound);
      }
      break;
    }

    if(start)
      break;
  }

  return curpos;
}
/*
   template <typename customType>
   bool get_NBT_value(NBT_struct *input, std::string TAG, customType *value)
   {
   for(unsigned i=0;i<input->values.size();i++)
   {
    if(input->values[i].name==TAG)
    {
   *value=*(customType*)input->values[i].value;
      return true;
    }
   }

   for(unsigned j=0;j<input->compounds.size();j++)
   {
    return get_NBT_value(&input->compounds[j], TAG, value);
   }
   return false;
   }
 */

std::uint8_t *get_NBT_pointer(NBT_struct *input, std::string TAG)
{
  std::uint8_t *pointer;

  for(unsigned i = 0; i < input->byte_arrays.size(); i++)
  {
    if(input->byte_arrays[i].name == TAG)
      return input->byte_arrays[i].data;
  }

  for(unsigned j = 0; j < input->compounds.size(); j++)
  {
    pointer = get_NBT_pointer(&input->compounds[j], TAG);
    if(pointer != 0)
      return pointer;
  }

  return 0;
}

NBT_list *get_NBT_list(NBT_struct *input, std::string TAG)
{
  NBT_list *pointer;

  for(unsigned i = 0; i < input->lists.size(); i++)
  {
    if(input->lists[i].name == TAG)
      return &input->lists[i];
  }

  for(unsigned j = 0; j < input->compounds.size(); j++)
  {
    pointer = get_NBT_list(&input->compounds[j], TAG);
    if(pointer != 0)
      return pointer;
  }

  return 0;
}


int dumpNBT_string(std::uint8_t *buffer, std::string name)
{
  int curpos = 0;
  putSint16(buffer, name.length());
  curpos += 2;

  for(unsigned int i = 0; i < name.length(); i++)
  {
    buffer[curpos] = name[i];
    curpos++;
  }

  return curpos;
}

int dumpNBT_value(NBT_value *input, std::uint8_t *buffer)
{
  int curpos = 0;
  buffer[curpos] = input->type;
  curpos++;
  curpos        += dumpNBT_string(&buffer[curpos], input->name);

  switch(input->type)
  {
  case TAG_BYTE:
    buffer[curpos] = *(char *)input->value;
    curpos++;
    break;

  case TAG_SHORT:
    putSint16(&buffer[curpos], *(std::int16_t *)input->value);
    curpos += 2;
    break;

  case TAG_INT:
    putSint32(&buffer[curpos], *(int *)input->value);
    curpos += 4;
    break;

  case TAG_LONG:
    putSint64(&buffer[curpos], *(std::int64_t *)input->value);
    curpos += 8;
    break;

  case TAG_FLOAT:
    putFloat(&buffer[curpos], *(float *)input->value);
    curpos += 4;
    break;

  case TAG_DOUBLE:
    putDouble(&buffer[curpos], *(double *)input->value);
    curpos += 8;
    break;

  case TAG_STRING:
    curpos += dumpNBT_string(&buffer[curpos], *(std::string *)input->value);
    break;
  }

  return curpos;
}

int dumpNBT_list(NBT_list *input, std::uint8_t *buffer)
{
  int curpos = 0;
  buffer[curpos] = TAG_LIST;
  curpos++;
  curpos        += dumpNBT_string(&buffer[curpos], input->name);

  buffer[curpos] = input->tagId;
  curpos++;

  putSint32(&buffer[curpos], input->length);
  curpos += 4;
  NBT_struct **structlist = (NBT_struct **)input->items;

  for(int i = 0; i < input->length; i++)
  {
    switch(input->tagId)
    {
    case TAG_BYTE:
      buffer[curpos] = *(char *)input->items[i];
      curpos++;
      break;

    case TAG_SHORT:
      putSint16(&buffer[curpos], *(std::int16_t *)input->items[i]);
      curpos += 2;
      break;

    case TAG_INT:
      putSint32(&buffer[curpos], *(int *)input->items[i]);
      curpos += 4;
      break;

    case TAG_LONG:
      putSint64(&buffer[curpos], *(std::int64_t *)input->items[i]);
      curpos += 8;
      break;

    case TAG_FLOAT:
      putFloat(&buffer[curpos], *(float *)input->items[i]);
      curpos += 4;
      break;

    case TAG_DOUBLE:
      putDouble(&buffer[curpos], *(double *)input->items[i]);
      curpos += 8;
      break;

    case TAG_STRING:
      curpos += dumpNBT_string(&buffer[curpos], *(std::string *)input->items[i]);
      break;

    case TAG_BYTE_ARRAY:
      curpos += dumpNBT_byte_array((NBT_byte_array *)input->items[i], &buffer[curpos], true);
      break;

    case TAG_COMPOUND:
      curpos += dumpNBT_struct(structlist[i], &buffer[curpos], true);
      break;
    }
  }

  return curpos;
}

int dumpNBT_byte_array(NBT_byte_array *input, std::uint8_t *buffer, bool list)
{
  int curpos = 0;

  if(!list)
  {
    buffer[curpos] = TAG_BYTE_ARRAY;
    curpos++;
    curpos        += dumpNBT_string(&buffer[curpos], input->name);
  }

  putSint32(&buffer[curpos], input->length);
  curpos += 4;
  memcpy(&buffer[curpos], input->data, input->length);
  curpos += input->length;

  return curpos;
}

int dumpNBT_struct(NBT_struct *input, std::uint8_t *buffer, bool list)
{
  int curpos = 0;

  if(!list)
  {
    buffer[curpos] = TAG_COMPOUND;
    curpos++;
    curpos        += dumpNBT_string(&buffer[curpos], input->name);
  }

  //Dump all values
  for(unsigned int i = 0; i < input->values.size(); i++)
  {
    curpos += dumpNBT_value(&input->values[i], &buffer[curpos]);
  }

  //Dump byte arrays
  for(unsigned int i = 0; i < input->byte_arrays.size(); i++)
  {
    curpos += dumpNBT_byte_array(&input->byte_arrays[i], &buffer[curpos]);
  }

  //Dump lists
  for(unsigned int i = 0; i < input->lists.size(); i++)
  {
    curpos += dumpNBT_list(&input->lists[i], &buffer[curpos]);
  }

  //Dump compounds
  for(unsigned int i = 0; i < input->compounds.size(); i++)
  {
    curpos += dumpNBT_struct(&input->compounds[i], &buffer[curpos]);
  }

  buffer[curpos] = 0x00; //TAG_END
  curpos++;

  //Total size
  return curpos;
}

bool freeNBT_list(NBT_list *input)
{
  //Free lists
  if(!input->length)
    return false;


  switch(input->tagId)
  {
    case TAG_BYTE:
      for(int j = 0; j < input->length; j++)
      {
        delete (char *)input->items[j];
      }
      delete [] (char **)input->items;
      break;

  case TAG_SHORT:
    for(int j = 0; j < input->length; j++)
    {
      delete (std::int16_t *)input->items[j];
    }
    delete[] (std::int16_t **)input->items;
    break;

  case TAG_INT:
    for(int j = 0; j < input->length; j++)
    {
      delete (int *)input->items[j];
    }
    delete[] (int **)input->items;
    break;

  case TAG_LONG:
    for(int j = 0; j < input->length; j++)
    {
      delete (std::int64_t *)input->items[j];
    }
    delete[] (std::int64_t **)input->items;
    break;

  case TAG_FLOAT:
    for(int j = 0; j < input->length; j++)
    {
      delete (float *)input->items[j];
    }
    delete[] (float **)input->items;
    break;

  case TAG_DOUBLE:
    for(int j = 0; j < input->length; j++)
    {
      delete (double *)input->items[j];
    }
    delete[] (double **)input->items;
    break;

  case TAG_STRING:
    for(int j = 0; j < input->length; j++)
    {
      delete (std::string *)input->items[j];
    }
    delete[] (std::string **)input->items;
    break;

  case TAG_COMPOUND:
    for(int j = 0; j < input->length; j++)
    {
      freeNBT_struct((NBT_struct *)input->items[j]);
      delete (NBT_struct *)input->items[j];
    }
    delete[] (NBT_struct **)input->items;
    break;

  case TAG_BYTE_ARRAY:
    for(int j = 0; j < input->length; j++)
    {
      NBT_byte_array *temparray = (NBT_byte_array *)input->items[j];
      delete[] temparray->data;
      delete temparray;
    }
    delete[] (NBT_byte_array **)input->items;
    break;
  }

  return true;
}

bool freeNBT_struct(NBT_struct *input)
{
  //Free all values
  for(unsigned int i = 0; i < input->values.size(); i++)
  {
    switch(input->values[i].type)
    {
    case TAG_BYTE:
      delete (char *)input->values[i].value;
      break;

    case TAG_SHORT:
      delete (std::int16_t *)input->values[i].value;
      break;

    case TAG_INT:
      delete (int *)input->values[i].value;
      break;

    case TAG_LONG:
      delete (std::int64_t *)input->values[i].value;
      break;

    case TAG_FLOAT:
      delete (float *)input->values[i].value;
      break;

    case TAG_DOUBLE:
      delete (double *)input->values[i].value;
      break;

    case TAG_STRING:
      std::string *temp_string = (std::string *)input->values[i].value;
      temp_string->clear();
      delete (std::string *)input->values[i].value;
      break;
    }
  }

  input->values.clear();

  //Free byte arrays
  for(unsigned int i = 0; i < input->byte_arrays.size(); i++)
  {
    delete[] input->byte_arrays[i].data;
  }

  for(unsigned int i = 0; i < input->lists.size(); i++)
  {
    freeNBT_list(&input->lists[i]);
  }

  input->byte_arrays.clear();
  input->lists.clear();

  //Free compounds
  for(unsigned int i = 0; i < input->compounds.size(); i++)
  {
    freeNBT_struct(&input->compounds[i]);
  }

  input->compounds.clear();

  return true;
}

//===========================================================================

std::string base36 (int i)
{
    static char x[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    std::string result;
    if (i < 0)
    {
        result = "-";
        i = -i;
    }

    if (i >= 36)
    {
        result += x[(i / 36) % 36];
        i -= ((i / 36) % 36) * 36;
    }

    result += x[i];

    return result;
}

struct nbt_chunk
{
    nbt_chunk() : block (16*16*16), data (16*16*16/2) {}
    std::vector<uint8_t> block;
    std::vector<uint8_t> data;
};


// c10t mcregion class

struct chunk_offset {
    uint32_t sector_number;
    uint8_t sector_count;
};

class region;

typedef boost::shared_ptr<region> region_ptr;

typedef std::vector<char> dynamic_buffer;

class region
{
public:
    enum
    {
        REGION_SIZE = 32,
        REGION_HEIGHT = 8,
        HEADER_RECORD_SIZE = 4,
        HEADER_SIZE = REGION_SIZE * REGION_SIZE * HEADER_RECORD_SIZE,
        RECORD_SIZE = 4096,
        PAGE_SIZE = 4096,
        CHUNK_MAX = 1024 * 128
    }
    constants;

private:
    fs::path path;
    boost::shared_array<char> header;

public:
    region(const fs::path& path);

    void read_header();
    inline unsigned int get_offset(unsigned int x, unsigned int y, unsigned int z) const;

    chunk_offset read_chunk_offset(unsigned int x, unsigned int y, unsigned int z) const;

    uint32_t read_data(int x, int y, int z, nbt_chunk& buffer);
    fs::path get_path();

    bool exists() const { return fs::exists(path); }
};

region::region(const fs::path& path)
 : path(path)
{
}

void region::read_header()
{
    header.reset(new char[HEADER_SIZE]);

    std::ifstream fp(path.string().c_str(), std::ios::binary);

    if (fp.fail()) {
      throw std::runtime_error(std::string("file not found (") + path.string() + ")");
    }

    fp.read(header.get(), HEADER_SIZE);

    if (fp.fail()) {
        throw std::runtime_error(std::string("cannot read header (") + path.string() + ")");
    }
}

inline unsigned int region::get_offset(unsigned int x, unsigned int y, unsigned int z) const
{
    return 4 * ((x&31) + (z&31) * REGION_SIZE);
}

chunk_offset region::read_chunk_offset(unsigned int x, unsigned int y, unsigned int z) const
{
    if (!header) {
      throw bad_region(path, "header has not been loaded");
    }

    int o = get_offset(x, y, z);

    uint8_t buf[HEADER_RECORD_SIZE];

    ::memcpy(reinterpret_cast<char*>(buf), &header[o], HEADER_RECORD_SIZE);

    chunk_offset co;

    co.sector_number = (buf[0] << 16) + (buf[1] << 8) + (buf[2]);
    co.sector_count = uint8_t(buf[3]);

    return co;
}

uint32_t region::read_data(int x, int y, int z, nbt_chunk& buffer)
{
    if (y <0 || y>15)
    {
        //std::cout << "y " << y << " out of range" << std::endl;
        //return 0;
        y = y & 15;
    }

    chunk_offset co = read_chunk_offset(x, y, z);
    if (co.sector_number == 0 && co.sector_count == 0)
        return 0;

    uint8_t buf[5];

    std::ifstream fp(path.string().c_str(), std::ios::binary);

    if (fp.fail()) {
      throw bad_region(path, "failed to open file");
    }

    fp.seekg(co.sector_number * PAGE_SIZE);

    std::cout << "Anvil chunk " << x << "  " << y << " " << z << " in " << path.string() << " resides in sector " << co.sector_number << " count " << (int)co.sector_count << std::endl;
    if (fp.fail()) {
      throw bad_region(path, "could not seek");
    }

    fp.read(reinterpret_cast<char*>(buf), 5);

    if (fp.fail()) {
      throw bad_region(path, "could not read chunk header");
    }

    uint32_t len = (buf[0]<<24) + (buf[1]<<16) + (buf[2]<<8) + (buf[3]);
    uint8_t  compression_scheme = buf[4];

    std::cout << len << " bytes, scheme " << (unsigned int)buf[4] << std::endl;
    if (compression_scheme != 2) {
      throw bad_region(path, "unsupported compression scheme");
    }

    dynamic_buffer in_buffer(len);
    fp.read(&*in_buffer.begin(), len);

    if (fp.fail()) {
      throw bad_region(path, "could not read chunk");
    }

    fp.close();

    dynamic_buffer nbt_buf (256000);
    uLongf tmp (nbt_buf.size());

    int result = uncompress((Bytef*)&*nbt_buf.begin(), &tmp,
               (Bytef*)&*in_buffer.begin(), in_buffer.size());

    if (result == Z_BUF_ERROR)
        throw bad_region(path, "buffer too small");
    if (result == Z_MEM_ERROR)
        throw bad_region(path, "out of memory");
    if (result == Z_DATA_ERROR)
        throw bad_region(path, "compressed data got corrupted");

    std::cout << "Uncompressed data is " << tmp << " bytes" << std::endl;

{
        std::stringstream name;
        name << "dump." << x<<"."<<z;
        std::ofstream d (name.str().c_str(), std::ios::binary);
        d.write((const char*)&*nbt_buf.begin(), tmp);
}
    NBT_struct nbt;
    TAG_Compound((uint8_t*)&*nbt_buf.begin(), &nbt, true);

/*
    const std::uint8_t* p = get_NBT_pointer(&nbt, "Blocks");
    if (p != 0)
    {
        std::copy(p, p + buffer.size(), &*buffer.begin());
        freeNBT_struct(&nbt);
    }
    else
        throw bad_region(path, "could not unpack chunk");
        */

    const char * hack(&*nbt_buf.begin());
    std::array<uint8_t, 12> s;
    s[0] = 1;
    s[1] = 'Y';
    s[2] = y;
    s[3] = 7;
    s[4] = 0;
    s[5] = 6;
    s[6] = 'B';
    s[7] = 'l';
    s[8] = 'o';
    s[9] = 'c';
    s[10]= 'k';
    s[11]= 's';

    const char* p = std::search(hack, hack+tmp, s.begin(), s.begin()+12);

    if (p < hack+tmp)
    {
        p+=16;
        std::copy(p, p + 16*16*16, &*buffer.block.begin());
    }
    else
    {
        std::stringstream name;
        name << "dump." << x << "." << y << "." << z;
        std::ofstream d (name.str(), std::ios::binary);
        d.write(hack,tmp);
        //std::ofstream d2 ("dump2", std::ios::binary);
        //d2.write(&*in_buffer.begin(), len);

        throw bad_region(path, "could not find 'Blocks'");
    }

    const char * s2 ("Data");
    const char* p2 = std::search(p+4096, hack+tmp, s2, s2+4);

    if (p2 < hack+tmp)
    {
        p2+=8;
        std::copy(p2, p2 + 16*16*8, &*buffer.data.begin());
    }

    return 0;
}

fs::path region::get_path() {
    return path;
}


//===========================================================================

} // anonymous namespace


struct anvil_generator::impl
{
    fs::path            savegame_;
    chunk_coordinates   origin_;
    boost::mutex        lock_;

    std::unordered_map<world_coordinates, nbt_chunk> cache_;

    impl(fs::path savegame, chunk_coordinates origin)
        : savegame_ (savegame)
        , origin_   (origin)
    {
    }

    chunk_height estimate_height (map_coordinates pos)
    {
        if (   pos.x >= chunk_world_limit.x
            || pos.y >= chunk_world_limit.y)
        {
            std::cout << pos << " is not legal!" << std::endl;
            throw std::logic_error("not a valid map position");
        }


        int mcx (  int(pos.x) - origin_.x);
        //int mcy (0);
        int mcz (-(int(pos.y) - origin_.y));

        boost::mutex::scoped_lock lock (lock_);
        std::stringstream name;
        name << "r2." <<        (int)std::floor(mcx / 32.)
                      << '.' << (int)std::floor(mcz / 32.)
                      << ".mca";

        region reg (savegame_ / fs::path("region") / name.str());

        try
        {
            reg.read_header();
            //chunk_offset co (reg.read_chunk_offset(mcx, mcy, mcz));
            //if (co.sector_number == 0 && co.sector_count == 0)
            //    return undefined_height;
        }
        catch (...)
        {
            return undefined_height;
        }
        return origin_.z + 16;
    }

    void generate (map_coordinates pos, area_data& dest)
    {
        if (   pos.x >= chunk_world_limit.x
            || pos.y >= chunk_world_limit.y)
        {
            std::cout << pos << " is not legal!" << std::endl;
            throw std::logic_error("not a valid map position");
        }
    }

    void generate (chunk_coordinates pos, chunk& dest)
    {
        if (   pos.x >= chunk_world_limit.x
            || pos.y >= chunk_world_limit.y
            || pos.z >= chunk_world_limit.z)
        {
            std::cout << pos << " is not legal!" << std::endl;
            throw std::logic_error("not a valid chunk position");
        }

        try
        {

        int mcx (   int(pos.x) - origin_.x);
        int mcy (   int(pos.z) - origin_.z);
        int mcz (- (int(pos.y) - origin_.y));

        world_coordinates mapped (pos.x, pos.y, pos.z);

        {
        boost::mutex::scoped_lock lock (lock_);

        if (cache_.count(mapped) == 0)
        {
            std::stringstream name;
            name << "r." <<        (int)std::floor(mcx / 32.)
                         << '.' << (int)std::floor(mcz / 32.)
                         ;

//            int t ((int)std::floor(mcz / 8.));
//            if ( t != 0)
//                name << '.' << t;

            name << ".mca";

            region reg(savegame_ / fs::path("region") / name.str());

            std::cout << "Reading anvil " << mcx << " " << mcy << " " << mcz << "   " << name.str() << " ..." << std::endl;

            reg.read_header();
            reg.read_data(mcx, mcy, mcz, cache_[mapped]);
            std::cout << "   ... succeeded" << std::endl;
        }

        const nbt_chunk& c (cache_[mapped]);

        size_t index (0);
        for (int y (0); y < 16; ++y)
        {
            for (int z (0); z < 16; ++z)
            {
                for (int x (0); x < 16; ++x)
                {
                    uint8_t mcb (c.block[index]);
                    uint8_t dat (c.data[index / 2]);

                    uint16_t type = mcb * 16;

                    // Air blocks sometimes have metadata, make sure air
                    // always ends up as zero.
                    if (type != 0)
                    {
                        if (index & 1)
                            type += dat & 0x0f;
                        else
                            type += dat / 16;
                    }

                    dest(x,15-z,y) = type;

                    ++index;
                }
            }
        }
        }
        }
        catch (bad_region& e)
        {
            std::cout << "Bad minecraft region: " << e.what() << std::endl;
        }
        catch (std::exception& e)
        {
            //std::cout << "Exception " << e.what() << std::endl;
        }
    }
};

anvil_generator::anvil_generator
                    (world& w, const ptree& config)
    : terrain_generator_i   (w, config)
    , pimpl_ (new impl(config.get<std::string>("path"),
                       config.get<world_coordinates>("origin")))
{ }

anvil_generator::~anvil_generator()
{ }

chunk_height
anvil_generator::estimate_height (map_coordinates xy) const
{
    return pimpl_->estimate_height(xy);
}

void anvil_generator::generate(chunk_coordinates pos, chunk& dest)
{
    pimpl_->generate(pos, dest);
}

} // namespace hexa

