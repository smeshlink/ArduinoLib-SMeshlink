#ifndef MISTRY_FEED_H
#define MISTRY_FEED_H

#include <Client.h>
#include <MistyDatastream.h>
#include <Printable.h>

class MistyFeed : public Printable
{
public:
  MistyFeed(unsigned long aID, MistyDatastream* aDatastreams, int aDatastreamsCount);

  virtual size_t printTo(Print&) const;
  unsigned long id() { return _id; };
  int size() { return _datastreamsCount; };
  MistyDatastream& operator[] (unsigned i) { return _datastreams[i]; };
protected:
  unsigned long _id;
  MistyDatastream* _datastreams;
  int _datastreamsCount;
};

#endif

