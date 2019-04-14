
#include "BorderParser.h"

int
slen(const char* str)
{
  int l;
  for (l = 0; str[l]; l++)
    ;
  return l;
}

int
needleIndex(char* str, const char* need, int startPos)
{
  int maxSearchIndex = slen(str) - slen(need);
  int needLen        = slen(need);
  for (int si = startPos; si <= maxSearchIndex; si++)
  {
    for (int ni = 0; str[si + ni] == need[ni]; ni++)
    {
      if (ni == needLen - 1)
        return si;
    }
  }
  return -1;
}

BorderParser::BorderParser(const char* b1, const char* b2, const char* nodata)
  : border1(b1)
  , border2(b2)
  , bSize(120)
{
  buffer = new char[bSize];
  data   = new char[bSize];
  count  = 0;
  for (int i = 0; i < bSize - 2; i++)
  {
    buffer[i] = '_';
  }

  buffer[bSize - 1] = '\0';

  for (int i = 0; i < bSize - 2; i++)
    data[i] = '\0';
  for (int i = 0; nodata[i]; i++)
    data[i] = nodata[i];

  bLen1 = slen(border1);
  bLen2 = slen(border2);
  upd();
}

BorderParser::~BorderParser()
{
}

bool
BorderParser::upd()
{
  count++;

  int bi1 = 0, bi2 = 0;

  bi1 = needleIndex(buffer, border1, 0);
  if (bi1 < 0)
    return false;

  bi1 = bi1 + bLen1;

  bi2 = needleIndex(buffer, border2, bi1);

  if (bi2 < 0)
    return false;
  for (int i = 0; i < bSize; i++)
    data[i] = '\0';
  for (int i = bi1; i < bi2; i++)
    data[i - bi1] = buffer[i];

  for (int i = 0; i < bi2 + bLen2; i++)
    buffer[i] = '_';

  return true;
}

bool
BorderParser::read(char c)
{
  for (int i = 0; i < bSize - 4; i++)
  {
    buffer[i] = buffer[i + 1];
  }
  buffer[bSize - 4] = c;

  return upd();
}

void
BorderParser::connect(BorderParser parser)
{
  buffer = parser.buffer;
}
