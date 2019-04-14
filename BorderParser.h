class BorderParser // get a stream of characters and searches for a value between two given substrings
{
private:
  int         count;

  const int   bSize;
  const char* border1;
  const char* border2;
  int         bLen1;
  int         bLen2;

public:
  // buffer
  char* buffer;
  // parsing result
  char* data;
  // update data, returns TRUE if data has been updated
  bool  upd();        
  // write 1 char to buffer and update data, returns TRUE if data has been updated
  bool  read(char c);
  // combines buffers of parsers
  void connect(BorderParser parser); 
  /*
  b1 - first border
  b2 - second border
  nodata - string that will be returned before parsing
  */
  BorderParser(const char* b1, const char* b2, const char* nodata);
  ~BorderParser();
  static bool parse(BorderParser& parser,
                    char*&        ans,
                    bool (*checkF)(),
                    char (*charF)())
  {
    if (checkF())
      if (parser.read(charF()))
      {
        ans = parser.data;
        return true;
      }
    return false;
  }
};
