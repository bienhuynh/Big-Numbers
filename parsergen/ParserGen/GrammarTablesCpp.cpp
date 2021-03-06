#include "stdafx.h"
#include "GrammarCode.h"

static const TCHAR *TABLETYPE = _T("static const TableType");

static const TCHAR *comment1 = 
_T("/****************************************************************************\\\n"
   "* Bitset containing 1-bits for compressed states and 0-bits for              *\n"
   "* uncompressed states. Used by ParserTable to determine how to               *\n"
   "* interpret the entry in the action-table                                    *\n"
   "* Number of bytes in bitset = (statecount-1)/8+1                             *\n"
   "\\****************************************************************************/\n")
  ;
static const TCHAR *comment2 = 
_T("/****************************************************************************\\\n"
   "* The action matrix holds the parse action(state,terminal)                   *\n"
   "* Used in LRParser::parserStep().                                            *\n"
   "* 2 different formats are used:Compressed and Uncompressed.                  *\n"
   "*                                                                            *\n"
   "* Compressed:When there is only 1 possible action in the state.              *\n"
   "*            or all actions are reduce by the same production.               *\n"
   "*   If bit 16 (0x8000) is 0, there is only one possible action which is      *\n"
   "*   encoded as:                                                              *\n"
   "*      #define   act0001  (void*)((action << 16) | (token&0x7fff)            *\n"
   "*   State index------+               |                |                      *\n"
   "*   Highorder 16 bits = action-------+                |                      *\n"
   "*   Loworder  15 bits = legal token-------------------+                      *\n"
   "*                                                                            *\n"
   "*   If bit 16 is 1, the loworder 15 bits is an index.                        *\n"
   "*      #define   act0002  (void*)((action<<16)  |  0x8000 | (index&0x7fff))  *\n"
   "*   State index------+               |                |        |             *\n"
   "*   Highorder 16 bits = action-------+                |        |             *\n"
   "*   Indicates that this is a multi-item-action-state--+        |             *\n"
   "*   Loworder  15 bits = index into compressedLasets------------+             *\n"
   "*                                                                            *\n"
   "* compressedLasets contains all unique legal lookahead-sets for all          *\n"
   "* multi-item-action-states.                                                  *\n"
   "* Number of bytes in each bitset = (terminalcount-1)/8+1                     *\n"
   "*                                                                            *\n"
   "* Uncompressed states:                                                       *\n"
   "*      TableType act0004[] =      { 3, 1, 2, 2,-2, 6, 1 };                   *\n"
   "*   State index------+              |  |  |                                  *\n"
   "*   Number of pairs in list---------+  |  |                                  *\n"
   "*   Legal input symbol-----------------+  |                                  *\n"
   "*   Action--------------------------------+                                  *\n"
   "*                                                                            *\n"
   "* If any uncompressed state has the same actionarray as a previous           *\n"
   "* generated uncompressed state, a #define is generated to save space.        *\n"
   "*                                                                            *\n"
   "*                                                                            *\n"
   "*   action = getAction(current state, current inputSymbol);                  *\n"
   "*                                                                            *\n"
   "*   action <  0   - Reduce by production p, p == -action.                    *\n"
   "*   action == 0   - Accept. Reduce by production 0.                          *\n"
   "*   action >  0   - Go to state s (=action),                                 *\n"
   "*                   and push [s,input,pos] to parser stack.                  *\n"
   "*                   Then read next symbol from input.                        *\n"
   "*   action == _ParserError - not found (=unexpected input).                  *\n"
   "\\****************************************************************************/\n")
  ;
static const TCHAR *comment3 = 
_T("/****************************************************************************\\\n"
   "* The successor matrix is used when the parser has reduced by prod A -> alfa *\n"
   "* Used by LRParser to find newstate = successor(state,A).                    *\n"
   "*                                                                            *\n"
   "*                  succ0007[] = { 3, 5,3 , 6,2, 8,5 };                       *\n"
   "*   Current state-------+         |  | |                                     *\n"
   "*   Number of pairs in list ------+  | |                                     *\n"
   "*   NonTerminal A--------------------+ |                                     *\n"
   "*   Go to this state-------------------+                                     *\n"
   "\\****************************************************************************/\n")
  ;
static const TCHAR *comment4 = 
_T("/****************************************************************************\\\n"
   "* The productionLength[] array is indexed by production number and holds     *\n"
   "* the number of symbols on the right side of each production.                *\n"
   "\\****************************************************************************/\n")
  ;
static const TCHAR *comment5 = 
_T("/****************************************************************************\\\n"
   "* The leftSide[] array is indexed by production number, and holds the        *\n"
   "* nonTerminal symbol on the left side of each production.                    *\n"
   "\\****************************************************************************/\n")
  ;

static const TCHAR *comment6 = 
_T("/****************************************************************************\\\n"
   "* The rightSide[] matrix is indexed by production number and holds           *\n"
   "* the right side symbols of each production.                                 *\n"
   "* Compressed and only used for debugging.                                    *\n"
   "\\****************************************************************************/\n")
  ;
static const TCHAR *comment7 = 
_T("/****************************************************************************\\\n"
   "* The symbolName[] array is indexed by symbol and holds                      *\n"
   "* the name of each symbol (terminal or nonTerminal).                         *\n"
   "* Used for debugging.                                                        *\n"
   "\\****************************************************************************/\n")
  ;

void GrammarTables::printCpp(MarginFile &output, bool useTableCompression) const {
  const bool   useShortTable = tableTypeIsShort();
  const TCHAR *tableType     = useShortTable ? _T("short") : _T("char");
  const int    tableTypeSize = getTableTypeSize();

  m_countTableBytes = 0;

  output.printf(_T("typedef %s TableType;\n\n"), tableType);

  if(!useTableCompression) {
    m_compressibleStateSet.clear();
  }
  const ByteArray stateTypeMap = getCompressedStateSetAsByteArray();

  output.printf(comment1);
  m_countTableBytes += printByteArray(output, _T("compressedSet"), stateTypeMap);
  output.printf(_T("\n"));

  output.printf(comment2);
  m_countTableBytes += printCompressedActionsCpp(output);
  m_countTableBytes += printUncompressedActionsCpp(output);

  m_countTableBytes += printActionMatrixCpp(output);

  m_countTableBytes += printSuccessorMatrixCpp(output);

  m_countTableBytes += printProductionLengthTableCpp(output);

  m_countTableBytes += printLeftSideTableCpp(output);

  m_countTableBytes += printRightSideTableCpp(output);

  m_countTableBytes += printSymbolNameTableCpp(output);

  int tableClassSize;
  if(useShortTable) {
    output.printf(_T("static const ParserShortTables %s_s("), m_tablesClassName.cstr());
    tableClassSize = sizeof(ParserShortTables);
  } else {
    output.printf(_T("static const ParserCharTables %s_s("), m_tablesClassName.cstr());
    tableClassSize = sizeof(ParserCharTables);
  }
  const int column = output.getCurrentLineLength()-1;
  output.printf(_T("compressedSet, compressedLasets\n"
                   "%*s,action, successor, productionLength, leftSide\n"
                   "%*s,rightSideTable, symbolName\n"
                   "%*s,%d, %d, %d, %d);\n\n")
                   ,column,_T(""),column,_T(""),column,_T("")
                   ,getTerminalCount(), getSymbolCount(), getProductionCount(), getStateCount());

  const int pointerSize = sizeof(void*);
  output.printf(_T("const ParserTables *%s = &%s_s;\n"), m_tablesClassName.cstr(), m_tablesClassName.cstr());
  output.printf(_T("// Size of %s_s: %d bytes. Size of %s:%d bytes\n")
               ,m_tablesClassName.cstr(), tableClassSize
               ,m_tablesClassName.cstr(), pointerSize);

  m_countTableBytes += tableClassSize + pointerSize;

  output.printf(_T("\n\n// Total size of table data:%s bytes.\n"), format1000(m_countTableBytes).cstr());
}

int GrammarTables::printActionMatrixCpp(MarginFile &output) const {
  const int stateCount = getStateCount();
  output.setLeftMargin(0);
  output.printf(_T("static const void *action[%d] = {\n"), stateCount);
  output.setLeftMargin(2);
  for(int s = 0; s < stateCount; s++) {
    output.printf(s > 0 ? _T(","):_T(" "));
    output.printf(_T("act%04d"), s);
    if(s % 10 == 9 || s == stateCount-1) {
      output.printf(_T("\n"));
    }
  }
  output.setLeftMargin(0);
  const int byteCount = wordAlignedSize(stateCount*sizeof(void*));
  output.printf(_T("}; // Size of table:%d bytes.\n\n"), byteCount);
  return byteCount;
}

// ---------------------------------------- Compressed actions ---------------------------------------------

class CompressedLookaheadMap : public TreeMap<BitSet, unsigned short> {
public:
  CompressedLookaheadMap() : TreeMap<BitSet, unsigned short>(bitSetCmp) {
  }
};

typedef Entry<BitSet, unsigned short> LASetEntry;

class BitSetWithIndex : public BitSet {
public:
  unsigned short m_index;
  BitSetWithIndex(const LASetEntry &e) : BitSet(e.getKey()), m_index(e.getValue()) {
  }
};

static int LASetIndexCmp(const BitSetWithIndex &s1, const BitSetWithIndex &s2) {
  return (int)s1.m_index - (int)s2.m_index;
}

int GrammarTables::printCompressedActionsCpp(MarginFile &output) const {
  CompressedLookaheadMap laSetMap;
  CompactIntArray        setIndex;

  const int stateCount = getStateCount();

  for(int state = 0; state < stateCount; state++) {
    if(isCompressibleState(state) && !isOneItemState(state)) {
      const BitSet laSet = getLookaheadSet(state) ;
      const unsigned short *indexp = laSetMap.get(laSet);
      unsigned short index;
      if(indexp != NULL) {
        index = *indexp;
      } else {
        index = (unsigned short)laSetMap.size();
        laSetMap.put(laSet, index);
      }
      setIndex.add(index);
    } else {
      setIndex.add(-1);
    }
  }
  const int laSetCount       = (int)laSetMap.size();
  int       laSetSize1State  = 0;
  int       byteCount        = 0;

  if(laSetCount == 0) {
    output.printf(_T("#define compressedLasets NULL\n\n"));
  } else {
    Array<BitSetWithIndex> laSetArray;
    for(Iterator<LASetEntry> it = laSetMap.entrySet().getIterator(); it.hasNext();) {
      laSetArray.add(it.next());
    }
    laSetMap.clear();
    laSetArray.sort(LASetIndexCmp);
    ByteArray laSetTotal;
    StringArray linePrefix;
    for(int i = 0; i < laSetCount; i++) {
      laSetTotal.append(bitSetToByteArray(laSetArray[i]));
      linePrefix.add(format(_T("%2d %3d tokens"), i, laSetArray[i].size()));
    }
    laSetSize1State = (int)laSetTotal.size() / laSetCount; // all laSets are the same size
    byteCount = printByteArray(output, _T("compressedLasets"), laSetTotal, laSetSize1State, &linePrefix);
    output.printf(_T("\n"));
  }

  for(int state = 0; state < stateCount; state++) {
    if(isCompressibleState(state)) {
      if(isOneItemState(state)) {
        const ParserAction  &pa         = m_stateActions[state][0];
        const short          action     = pa.m_action;                       // positive or negative
        const unsigned short token      = pa.m_token;
        const String         comment    = (action <= 0) 
                                        ? format(_T("reduce by %d on %s"), -action, getSymbolName(pa.m_token))
                                        : format(_T("shift to %d on %s"),   action, getSymbolName(pa.m_token));
        output.printf(_T("#define act%04d (void*)0x%08x    /* %-40s */\n"), state, (action<<16) | token, comment.cstr());
      } else {
        const short          action     = m_stateActions[state][0].m_action; // always negative
        const unsigned short index      = setIndex[state];
        const unsigned short byteIndex  = index * laSetSize1State;
        const String         comment    = format(_T("reduce by %d on tokens in set[%d]"), -action, index);
        output.printf(_T("#define act%04d (void*)0x%08x    /* %-40s */\n"), state, (action<<16) | byteIndex | 0x8000, comment.cstr());
      }
    }
  }
  output.printf(_T("\n"));

  return byteCount;
}

// ---------------------------------------- Uncompressed actions ---------------------------------------------

static unsigned long actionArrayHash(const ActionArray &a) {
  return a.hashCode();
}

static int actionArrayCmp(const ActionArray &a1, const ActionArray &a2) {
  return (a1 == a2) ? 0 : 1;
}

class ActionArrayMap : public HashMap<ActionArray, unsigned short> {
public:
  ActionArrayMap() : HashMap<ActionArray, unsigned short>(actionArrayHash, actionArrayCmp, 1000) {
  }
};

int GrammarTables::printUncompressedActionsCpp(MarginFile &output) const {
  const int stateCount    = getStateCount();
  const int tableTypeSize = getTableTypeSize();
  int       byteCount     = 0;

  m_uncompressedStateBytes = 0;
  ActionArrayMap actionMap;
  StringArray defines;
  for(int state = 0; state < stateCount; state++) {
    if(!isCompressibleState(state)) {
      const ActionArray &a = m_stateActions[state];
      const unsigned short *ps = actionMap.get(a);
      if(ps) {
        defines.add(format(_T("act%04d act%04d /* Saved %3d bytes */"), state, *ps, wordAlignedSize(tableTypeSize * (2*(int)a.size()+1))));
      } else {
        byteCount += printUncompressedActionArrayCpp(output, state);
        actionMap.put(a, state);
      }
    }
  }
  output.printf(_T("\n"));
  output.printf(_T("// Total size of uncompressed actNNNN tables:%d bytes.\n\n"), byteCount);
  if(defines.size() > 0) {
    for(size_t i = 0; i < defines.size(); i++) {
      output.printf(_T("#define %s\n"), defines[i].cstr());
    }
    output.printf(_T("\n"));
  }
  return byteCount;
}

int GrammarTables::printUncompressedActionArrayCpp(MarginFile &output, const unsigned int state) const {
  const ActionArray &actions       = m_stateActions[state];
  const int          actionCount   = (int)actions.size();

  output.setLeftMargin(0);
  output.printf(_T("%s act%04d[] = { %3d"), TABLETYPE, state, actionCount);
  for(int a = 0; a < actionCount; a++) {
    const ParserAction &pa = actions[a];
    output.printf(_T(",%4d,%4d"), pa.m_token, pa.m_action);
    if(a % 5 == 4 && a != actionCount-1) {
      output.printf(_T("\n"));
      output.setLeftMargin(40);
    }
  }
  output.printf(_T("};"));
  output.setLeftMargin(0);
  output.printf(_T("\n"));
  return wordAlignedSize((2*actionCount + 1) * getTableTypeSize());
}

int GrammarTables::printSuccessorMatrixCpp(MarginFile &output) const {
  output.printf(comment3);
  const int      stateCount    = getStateCount();
  const int      tableTypeSize = getTableTypeSize();
  int            byteCount     = 0;
  BitSet         hasSuccessor(stateCount);
  ActionArrayMap successorMap;
  StringArray    defines;

  for(int state = 0; state < stateCount; state++) {
    const ActionArray &succlist = m_stateSucc[state];
    const int          count    = (int)succlist.size();
    if(count > 0) {
      const unsigned short *ps = successorMap.get(succlist);
      if(ps) {
        defines.add(format(_T("succ%04d succ%04d  /* Saved %3d bytes */"), state, *ps, wordAlignedSize(tableTypeSize * (2*count+1))));
      } else {
        byteCount += printSuccessorArrayCpp(output, state);
        successorMap.put(succlist, state);
      }
      hasSuccessor += state;
    }
  }
  
  output.printf(_T("\n"));
  output.printf(_T("// Total size of all succNNNN tables:%d bytes.\n\n"), byteCount);

  if(defines.size() > 0) {
    for(size_t i = 0; i < defines.size(); i++) {
      output.printf(_T("#define %s\n"), defines[i].cstr());
    }
    output.printf(_T("\n"));
  }

  output.setLeftMargin(0);

  output.printf(_T("%s *successor[%d] = {\n"), TABLETYPE, stateCount);
  output.setLeftMargin(2);
  for(int state = 0; state < stateCount; state++) {
    output.printf(state > 0 ? _T(","):_T(" "));
    if(hasSuccessor.contains(state)) {
      output.printf(_T("succ%04d"), state);
    } else {
      output.printf(_T("NULL    "));
    }
    if(state % 10 == 9 || state == stateCount-1) {
      output.printf(_T("\n"));
    }
  }
  output.setLeftMargin(0);
  const int succTableSize = wordAlignedSize(sizeof(void*) * stateCount);

  output.printf(_T("}; // Size of pointertable:%d bytes.\n\n"), succTableSize);

  return byteCount + succTableSize;
}

int GrammarTables::printSuccessorArrayCpp(MarginFile &output, unsigned int state) const {
  const ActionArray &succlist = m_stateSucc[state];
  const int          count    = (int)succlist.size();
  
  output.setLeftMargin(0);
  output.printf(_T("%s succ%04d[] = { %2d"), TABLETYPE, state, count);
  char *delim = "";
  for(int a = 0; a < count; a++) {
    const ParserAction &pa = succlist[a];
    output.printf(_T(",%4d,%4d"), pa.m_token, pa.m_action);
    if(a % 5 == 4 && a != count-1) {
      output.printf(_T("\n"));
      output.setLeftMargin(40);
    }
  }
  output.setLeftMargin(0);
  output.printf(_T("};\n"));

  return wordAlignedSize(getTableTypeSize() * (2 * count + 1));
}

int GrammarTables::printProductionLengthTableCpp(MarginFile &output) const {
  output.printf(comment4);

  const int productionCount = getProductionCount();

  output.printf(_T("static const unsigned char productionLength[%d] = {\n"), productionCount);
  output.setLeftMargin(2);
  for(int p = 0; p < productionCount; p++) {
    const int l = m_productionLength[p];
    if(p % 10 == 0) {
      output.printf(_T("/* %3d */ "), p);
    }
    output.printf(_T(" %3d"), l);
    if(p < productionCount-1) {
      output.printf(_T(","));
    }
    if(p % 10 == 9) {
      output.printf(_T("\n"));
    }
  }
  output.setLeftMargin(0);

  const int byteCount = wordAlignedSize(productionCount*sizeof(char));

  output.printf(_T("\n}; // Size of table:%d bytes.\n\n"), byteCount);

  return byteCount;
}

int GrammarTables::printLeftSideTableCpp(MarginFile &output) const {
  output.printf(comment5);

  const int productionCount = getProductionCount();
  output.printf(_T("%s leftSide[%d] = {\n"), TABLETYPE, productionCount);
  output.setLeftMargin(2);

  for(int p = 0; p < productionCount; p++) {
    int l = m_left[p];
    if(p % 10 == 0) {
      output.printf(_T("/* %3d */ "), p);
    }
    output.printf(_T(" %3d"), l);
    if(p < productionCount-1) {
      output.printf(_T(","));
    }
    if(p % 10 == 9) {
      output.printf(_T("\n"));
    }
  }
  output.setLeftMargin(0);

  const int byteCount = wordAlignedSize(productionCount * getTableTypeSize());
  output.printf(_T("\n}; // Size of table:%d bytes.\n\n"), byteCount);

  return byteCount;
}

int GrammarTables::printRightSideTableCpp(MarginFile &output) const {
  output.printf(comment6);

  const int productionCount = getProductionCount();
  int       totalItemCount  = 0;

  for(int p = 0; p < productionCount; p++) {
    totalItemCount += (int)m_rightSide[p].size();
  }
  char *delim = " ";
  output.printf(_T("%s rightSideTable[%d] = {\n"), TABLETYPE, totalItemCount);
  output.setLeftMargin(2);
  for(int p = 0; p < productionCount; p++) {
    const CompactIntArray &r = m_rightSide[p];
    if(r.size() == 0) {
      continue;
    }
    output.printf(_T("/* %3d */ "), p);
    for(size_t i = 0; i < r.size(); i++, delim = ",") {
      output.printf(_T("%s%3d"), delim, r[i]);
    }
    output.printf(_T("\n"));
  }
  output.setLeftMargin(0);

  const int byteCount = wordAlignedSize(totalItemCount * getTableTypeSize());
  output.printf(_T("}; // Size of table:%d bytes.\n"), byteCount);
  output.printf(_T("\n"));

  return byteCount;
}

int GrammarTables::printSymbolNameTableCpp(MarginFile &output) const {
  output.printf(comment7);

  const int terminalCount   = getTerminalCount();
  const int symbolCount     = getSymbolCount();
  int       stringByteCount = 0;
  output.printf(_T("static const TCHAR *symbolName[%d] = {\n"), symbolCount);
  output.setLeftMargin(2);
  for(int s = 0; s < symbolCount; s++) {
    const String &name = m_symbolName[s];
    output.printf(_T("/* %3d */ _T(\"%s\")"), s, name.cstr());
    stringByteCount += wordAlignedSize((int)name.length() + 1);
    if(s < symbolCount-1) {
      output.printf(_T(","));
    }
    output.printf(_T("\n"));
  }
  output.setLeftMargin(0);

  const int tableByteCount = symbolCount * sizeof(char*);
  output.printf(_T("}; // Total size of strings:%d bytes, size of pointertable:%d bytes.\n\n"), stringByteCount, tableByteCount);

  return stringByteCount + tableByteCount;
}
