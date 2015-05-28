#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>
#include <pthread.h>
#include <algorithm>
#include <semaphore.h>
#include <cctype>
#include "pocker.h"
#include "arrays.h"
using namespace std;


template<class D>
class DataBufferPool
{
private:
	sem_t hEmpty;
	sem_t hUseful;
	pthread_mutex_t csDataChange;
	int iMaxNum;            //??????
	int iBufferSize;	 //??????????
	int iRead;                //???
	int iWrite;	 //???
	D **pData;	 //??????
	int iErrorNum;  /*         1:????,?????
					*/

public:
	DataBufferPool(int maxnum, int bufsize)
	{
		iMaxNum = maxnum;
		iBufferSize = bufsize;
		iRead = iWrite = 0;
		pData = NULL;
		int res = 0;
		res = sem_init(&hEmpty, 0, iMaxNum);  //???????
		if (res != 0)
		{
			iErrorNum = 2;
		}
		res = sem_init(&hUseful, 0, 0);	 //????????
		if (res != 0)
		{
			iErrorNum = 2;
		}
		pData = new D*[iMaxNum];	 //??????
		for (int i = 0; i<iMaxNum; i++)
		{
			pData[i] = new D[iBufferSize];	 //??????
			if (pData[i] == NULL)
			{
				iErrorNum = 1;
			}
		}
		res = pthread_mutex_init(&csDataChange, NULL);
		if (res != 0)
		{
			iErrorNum = 3;
		}
	}
	~DataBufferPool()
	{
		for (int i = 0; i<iMaxNum; i++)
		{
			delete[](pData[i]);
		}
		delete[]pData;

		sem_destroy(&hEmpty);
		sem_destroy(&hUseful);
		pthread_mutex_destroy(&csDataChange);

	}
	bool WriteData(void *data, int datasize)
	{

		sem_wait(&hEmpty);  //??????
		pthread_mutex_lock(&csDataChange);//???????,???????
		iWrite %= iMaxNum;
		//////////////////////////////////////////////////////////////////////
		//printf("produce %d \n",iWrite);//???????,???
		//////////////////////////////////////////////////////////////////////
		if (datasize > iBufferSize) return false;
		memcpy((void *)pData[iWrite], (void *)data, datasize*sizeof(D));
		++iWrite;
		pthread_mutex_unlock(&csDataChange);
		sem_post(&hUseful);
		return true;
	}
	bool ReadData(void *data, int datasize)
	{
		sem_wait(&hUseful);//??????
		pthread_mutex_lock(&csDataChange);
		iRead %= iMaxNum;
		/////////////////////////////////////////////////////////////////////
		//printf("*********consume  %d \n",iRead);//???????,???
		/////////////////////////////////////////////////////////////////////
		if (datasize > iBufferSize) return false;
		memcpy((void *)data, (void *)pData[iRead], datasize*sizeof(D));
		++iRead;
		pthread_mutex_unlock(&csDataChange);
		sem_post(&hEmpty);
		return true;
	}
};
void *thread_function(void *arg);

typedef struct _DataSturct
{
	int socknum;
	void *pSome;
}DataStruct;

#define MAXSEMNUM 10
int lunSum = 0;//use in action ,clear in addCard function
#define LUNMAX 5
#define RAISEMAX_CRAZZY 4
#define RAISEMAX_INTEL 3
#define CALLMAX_CRAZY 2
#define CALLMAX_INTEL 2
#define CHECKMAX_INTEL 1
#define TWOCARDMAXRAISENUM 2
#define MAXJU      600
bool bOver = false;

typedef struct _GameHis
{
	string ID;
	int jetton;
	int money;
	int bet;
	string status;

	int frontbet;
	int foldnum;
	int winnum;


}GameHis;



typedef struct _inquireInfo{
	int noFlopNum, checkNum, callNum, raiseNum, allinNum, blindNum, foldNum, noFlodLeastJetton;
	vector<int> allInPlayerBet;
}inquireInfo;


std::string cardpool[4][13] ={
        {"C2","C3","C4","C5","C6","C7","C8","C9","C1","CJ","CQ","CK","CA"},
        {"D2","D3","D4","D5","D6","D7","D8","D9","D1","DJ","DQ","DK","DA"},
        {"H2","H3","H4","H5","H6","H7","H8","H9","H1","HJ","HQ","HK","HA"},
        {"S2","S3","S4","S5","S6","S7","S8","S9","S1","SJ","SQ","SK","SA"}
};
int sixcardsixselect[6][5]={
  {0,1,2,3,4},
  {0,1,2,3,5},
  {0,1,2,4,5},
  {0,1,3,4,5},
  {0,2,3,4,5},
  {1,2,3,4,5}
};
int sevencardtwentyoneselect[21][5] = {
  { 0, 1, 2, 3, 4 },
  { 0, 1, 2, 3, 5 },
  { 0, 1, 2, 3, 6 },
  { 0, 1, 2, 4, 5 },
  { 0, 1, 2, 4, 6 },
  { 0, 1, 2, 5, 6 },
  { 0, 1, 3, 4, 5 },
  { 0, 1, 3, 4, 6 },
  { 0, 1, 3, 5, 6 },
  { 0, 1, 4, 5, 6 },
  { 0, 2, 3, 4, 5 },
  { 0, 2, 3, 4, 6 },
  { 0, 2, 3, 5, 6 },
  { 0, 2, 4, 5, 6 },
  { 0, 3, 4, 5, 6 },
  { 1, 2, 3, 4, 5 },
  { 1, 2, 3, 4, 6 },
  { 1, 2, 3, 5, 6 },
  { 1, 2, 4, 5, 6 },
  { 1, 3, 4, 5, 6 },
  { 2, 3, 4, 5, 6 }
};

class Player
{

public:
	Player(string str);
	void init();
	int setSeat(char *);
	int  getSeatPos(){ return m_iSeatPos; }
	vector<string> getSeatInfo() { vector<string> seat; for (int i = 0; i<m_iPlayerNum; i++) seat.push_back(m_sSeatInfo[i]); return seat; }
	void setCard(string card);
	string getCard(int index) { if (index < m_iCardNum) return m_sCard[index]; else return ""; }
	string getAction();
	int getPlayNum() { return m_iPlayerNum; }
	int getCardNum() { return m_iCardNum; }
	vector<int> getMaxCardType();
	void initCardType();
	GameHis getPlayerHis(string &str);
	void setBlindInfo(vector<string> blindInfo);

	int getMyRemJetton(){ return m_iRemainedJetton; }
	void deleaseMyJetton(int lease){ m_iRemainedJetton = m_iMyJetton - lease; }
	void setMyJetton(int jetton){ m_iMyJetton = jetton; }
    int getChuShiJetton(){ return m_iChuShiJetton;}
        int  getMyMoney(){ return m_iMoney[m_iSeatPos]; }
        void setHaveBet(int bet) { m_iHaveBet = bet; m_iRemainedJetton = m_iMyJetton - bet; }
        int getHaveBet(){ return m_iHaveBet; }
        int getLeastBet() { int ifrontBet = m_iFrontHaveBet - m_iHaveBet; return (ifrontBet > 0 ? ifrontBet : 0); }

	bool firstTwoCardIsPair(){ return m_sCard[1][1] == m_sCard[0][1]; }
	int getRaiseLeast(){ return m_iRaiseLeastBet; }
	//  void clearHist(){ gamehist.clear();}//zhi ji lu yi lun history

	bool getCardStatus(){ int i = getCardNum(); if (i == 2 || i >= 5) return false; else return true; }

	int addCard(char *info);
	void blind(char *info);
	inquireInfo inquire(char *info);
	void showdown(char *downmsg);

	int getErrorNum(){ return m_iError; }
	int getLunJiShu() { return m_iLunJiShu; }
	int getTotalBet() {return m_iTotalBet;}
	bool getLastPlayerBeginRaiseStatus(){ return m_bLastPlayerBeginRaise;}
	void setLastPlayerBeginRaiseStatus(bool status){ m_bLastPlayerBeginRaise = status;}

	void pot_win(char *info);
	void notify(char *info);

	int getWholeLevel(){ return m_iLevelWhole;}
	int getLastRaiseLevel()
	{
	  if(m_strFrontRaisePlayerID == "") return 0;
	  if( m_mLevel.find(m_strFrontRaisePlayerID) == m_mLevel.end())
	    return 0;
	  else
	    return m_mLevel[m_strFrontRaisePlayerID];
	}
	int getSumJetton(){ return m_iSumJettonBegin;}

	int getRemainderLevel();

	void handAllInPlayer(inquireInfo &inqInfo);
	int getWinRate() {return m_iWinRate;}

	bool getFirstInqurieStatus(){ return m_bInquireFisrt;}
	bool setFirstInquireFalse() { m_bInquireFisrt = false;}
	vector<vector<int> > getCardTypeSum(){ return  m_vCardTypeSum;	}
public:
	bool m_bChangeFrontHaveBet;
	int strConvertInt(char str);
	int gHisPlayNum;
private:
	//  string m_sCard[7];//0,1 our card , 2 3 4 5 6 public card
	vector<string> m_sCard;
	//  string m_sSeatInfo[8];//0:button, 1:small blind 2:big blind 3 4 5 6 7 other player
	vector<string> m_sSeatInfo;
	int m_iMoney[8];
	int m_iJetton[8];
	int m_iCardNum;
	int m_iPlayerNum;

	int m_iLeastBet;

	int m_iRaiseLeastBet;
	int m_iRemainedJetton;
	int m_iMyJetton;//kai si wo de chou ma
	int m_iHaveBet;//wo yi jing jia de zu
	int m_iFrontHaveBet;//pai zuo shang zai wo zhi qian jia de zui duo de zu,ru guo wo gen zu de hua
	//zhe m_iFrontHaveBet-m_iHaveBet wei wo ying gai jia de zu
	int m_iSeatPos;//wo de wei zhi
	string m_sMyID;//wo de ID

	vector<int> m_vCardType;

	enum OperateType
	{
		ADDCARD, BLIND, INQUERE,POTWIN
	};

	void slipInfo(char *info, vector<string> &vData);
	char* removeHead(char *seatInfo, string &rmData);
	void operation(OperateType oType, char *info);
	// vector<GameHis> gamehist;

	int m_iError;//1: card error  2:atoi error
	//tong ji quan ju xin xi
	int m_iLunJiShu;
	int m_iSumJettonBegin;
	int m_iChuShiJetton;
	int m_iTotalBet;
	bool m_bLastPlayerBeginRaise;//the end player of the front raise player ,true impress the last player front the raise player

	map<string,int> m_mHis[5];

	map<string , size_t> m_mFoldNum;
	bool m_bHaveNotify;

	map<string ,int> m_mLevel;
	void setLevel();
	int m_iLevelWhole;

	vector<string> m_strNofoldPlayID;
	string m_strFrontRaisePlayerID;

	inquireInfo m_iInqInfo;
	int  m_iWinRate;
	bool m_bInquireFisrt ;//first inquire
	vector<vector<int> > m_vCardTypeSum;
private:
	int calculateRank(vector<string> vcard);
	int fenSearch(int *A , int low , int high, int target);
	vector<int> StatisticsCard(vector<string> card);
};

Player::Player(string str)
{
	init();
	m_sMyID = str;
	m_iLunJiShu = 1;
	m_iSumJettonBegin = 0;
	m_iChuShiJetton = 0;
	for(int i=0;i<5;i++)
	  m_mHis[i].clear();

	m_mFoldNum.clear();
}

int Player::fenSearch(int *A , int low , int high, int target)
{
    while(low<=high)
    {
                int mid=(low+high)/2;
                if(A[mid]>target)
                        high=mid-1;
                else if(A[mid]<target)
                        low=mid+1;
                else//findthetarget
                        return mid;
    }
    return -1;
}
void Player::init()
{
	m_sCard.clear();
	for (int i = 0; i<8; i++)
	{

		m_iMoney[i] = 0;
		m_iJetton[i] = 0;
	}
	m_sSeatInfo.clear();
	m_iCardNum = 0;
	m_iPlayerNum = 0;


	m_iRemainedJetton = 0;
	m_iRaiseLeastBet = 0;
	m_iLeastBet = 0;
	m_iMyJetton = 0;
	m_iHaveBet = 0;
	m_iFrontHaveBet = 0;
	m_iSeatPos = 0;
	m_vCardType.clear();
	m_iError = 0;
	m_iTotalBet = 0;

	m_bHaveNotify = false;
	m_strFrontRaisePlayerID = "";
	m_iWinRate = 0;

}
void Player::slipInfo(char *info, vector<string> &vData) //end of " " example "bbbb 1111 2222 \n"
{
	char *spacepos = NULL, *infotemp = info;
	string s = "";
	while (*infotemp != '\0' && (NULL != (spacepos = strchr(infotemp, ' '))))
	{
		*spacepos = '\0';
		s = infotemp;
		vData.push_back(s);
		*spacepos = ' ';
		infotemp = spacepos + 1;
	}

}

char* Player::removeHead(char *seatInfo, string &rmData)
{
	char *posInfo = NULL;
	if (NULL == (posInfo = strstr(seatInfo, rmData.c_str()))) return NULL;
	return posInfo + strlen(rmData.c_str());
}

void Player::operation(OperateType oType, char *info)
{

	char *findbegin, *findend, *findtemp;
	char operateData[2048] = { 0 };
	memcpy(operateData, info, strlen(info) + 1);
	findbegin = operateData;
	vector<string > vData;
	string s = "";
	findtemp = strchr(findbegin, '\n');
	findbegin = findtemp + 1;
	while (*findbegin != '/')
	{
		s = "";
		vData.clear();
		findend = strchr(findbegin, '\n');
		*findend = '\0';

		slipInfo(findbegin, vData);
		*findend = '\n';
		findbegin = findend + 1;
		if (oType == ADDCARD)
		{
			if (vData[0][0] == 'S' || vData[0][0] == 'H' || vData[0][0] == 'C' || vData[0][0] == 'D')
			{
				setCard(s + vData[0][0] + vData[1][0]);
			}
			else m_iError = 1;
		}
		else if (oType == BLIND)
		{
			setBlindInfo(vData);
		}
		else
		{
			printf("operation error!!");

		}
	}

}

void Player::setLevel()
{
  map<string, size_t>::iterator w = m_mFoldNum.begin();

  for(;w != m_mFoldNum.end();w++)
    {
      if(w->second >= 40) m_mLevel[w->first] = -1;
      else if(w->second <=25 ) m_mLevel[w->first] = 1;
      else m_mLevel[w->first] = 0;
      printf("%s:foldNum=%d level=%d\n",(w->first).c_str(),w->second,m_mLevel[w->first]);
    }
   m_iLevelWhole = 0;
   map<string, int>::iterator l = m_mLevel.begin();
  for(; l!=m_mLevel.end();++l)
    {
      if(l->first != m_sMyID)
        m_iLevelWhole += l->second;
    }

  if(m_iLevelWhole > 0) m_iLevelWhole = 1;
  else if(m_iLevelWhole < 0) m_iLevelWhole = -1;
  else m_iLevelWhole = 0;
  printf("whole Level=%d\n",m_iLevelWhole);
}
int Player::getRemainderLevel()
{
  int lev=0;
  for(int i=0;i<m_strNofoldPlayID.size();i++)
    {
      lev += m_mLevel[m_strNofoldPlayID[i]];
    }
  return min(max(-1,lev),1);
}
int Player::addCard(char *info)
{
	if (*info == 'h')
	  {
	    m_iCardNum = 2;
	    if((m_iLunJiShu-1) %50 == 0)
	      {
		printf("set Level ...............................................\n");
		m_mLevel.clear();
		setLevel();
		m_mFoldNum.clear();

	      }
	  }

	else if (*info == 'f')
		m_iCardNum = 5;
	else if (*info == 't')
		m_iCardNum = 6;
	else if (*info == 'r')
		m_iCardNum = 7;
	else
	{
		printf("add card error");
		return -1;
	}

	operation(ADDCARD, info);

	if (m_iCardNum == 2 || m_iCardNum >= 5)
	{
		initCardType();
		m_bInquireFisrt = true;

	}

	return 0;
}
void Player::setCard(string card)
{
	m_sCard.push_back(card);
	//  printf("%s\n",card.c_str());
}
void Player::setBlindInfo(vector<string> blindInfo)
{
	printf("blindInfo[1]:%s\n", blindInfo[1].c_str());
	try
	{
		if (blindInfo[0].find(m_sSeatInfo[1]) != string::npos) m_iRaiseLeastBet = atoi(blindInfo[1].c_str()) * 2;

		if (string::npos == blindInfo[0].find(m_sMyID)) return;

		m_iHaveBet = atoi(blindInfo[1].c_str());
		deleaseMyJetton(m_iHaveBet);
	}
	catch (...)
	{
		m_iError = 2;
	}



}

int Player::setSeat(char *seatInfo)
{
	init();

	string sHead[3] = { "button: ", "small blind: ", "big blind: " };
	char *findbegin, *findend, *findtemp;
	findbegin = seatInfo;
	vector<string > vData;
	findtemp = strchr(findbegin, '\n');
	findbegin = findtemp + 1;
	while (*findbegin != '/')
	{
		vData.clear();
		if (*findbegin == 'b' && *(findbegin + 1) == 'u')
			findbegin = findbegin + sHead[0].size();
		else if (*findbegin == 's')
			findbegin = findbegin + sHead[1].size();
		else if (*findbegin == 'b' && *(findbegin + 1) == 'i')
			findbegin = findbegin + sHead[2].size();

		findend = strchr(findbegin, '\n');
		*findend = '\0';

		slipInfo(findbegin, vData);
		*findend = '\n';
		findbegin = findend + 1;

		m_sSeatInfo.push_back(vData[0]);

		if(m_mLevel.find(vData[0]) == m_mLevel.end())
		  m_mLevel[vData[0]] = 0;

		printf("setSeat:%d _ ID=%s jetton=%s money=%s\n",m_iPlayerNum,vData[0].c_str(), vData[1].c_str(),vData[2].c_str());
		try
		{
			m_iMoney[m_iPlayerNum] = atoi(vData[2].c_str());
			if (m_iLunJiShu == 1)
			{
				m_iSumJettonBegin += m_iMoney[m_iPlayerNum];
				m_iChuShiJetton = m_iMoney[m_iPlayerNum];

			}
			m_iJetton[m_iPlayerNum] = atoi(vData[1].c_str());
			if (vData[0] == m_sMyID)
			{
				m_iMyJetton = atoi(vData[1].c_str());
				m_iSeatPos = m_iPlayerNum;
			}
			++m_iPlayerNum;
		}
		catch (...)
		{
			m_iError = 2;
		}

	}

	gHisPlayNum = m_iPlayerNum;
	++m_iLunJiShu;
	return m_iPlayerNum;


}
void Player::blind(char *info)
{
	operation(BLIND, info);
}

void Player::showdown(char *downmsg)
{

}

void Player::pot_win(char *info)
{

}
int Player::strConvertInt(char str)
{
	int i = 0;
	switch (str)
	{
	case '1':
		i = 10;
		break;
	case 'J':
		i = 11;
		break;
	case 'Q':
		i = 12;
		break;
	case 'K':
		i = 13;
		break;
	case 'A':
		i = 14;
		break;
	default:
		i = str - '0';
	}
	return i;
}

int Player::calculateRank(vector<string> vcard)
{
          int card[5]={0};
          for(int m=0;m <vcard.size();m++)
          {

                  int cardpoint = strConvertInt(vcard[m][1]);

		  char prime = primes[cardpoint-2];
		  card[m] = prime;
		  card[m] += (cardpoint-2)<<8;
		  if(vcard[m][0] == 'C')
			  card[m] += 1<<15;
		  else if(vcard[m][0] == 'D')
			  card[m] += 1<<14;
		  else if(vcard[m][0] == 'H')
			  card[m] += 1<<13;
		  else if(vcard[m][0] == 'S')
			  card[m] += 1<<12;
		  else
			  printf("card collor error\n");
		  card[m] += 1<<(cardpoint+14);
	  }
	  int q=0;
	  int mingchi;
	  if( card[0] & card[1] &card[2] & card[3] & card[4] & 0xF000)
	  {
		  q = (card[0] | card[1]  | card[2] | card[3] | card[4])>>16;
		  printf("Tong Hua Ming Chi is %d\n",flushes[q]);
	  }
	  else
	  {
		  q = (card[0] | card[1]  | card[2] | card[3] | card[4])>>16;
		  mingchi = unique5[q];
		  if(mingchi != 0)
			  printf("Fei tong hua ming chi %d\n",mingchi);
		  else
		  {
		  q = (card[0] & 0xFF)*(card[1] & 0xFF)*(card[2] & 0xFF)*(card[3] & 0xFF)*(card[4] & 0xFF);
		  int index = fenSearch(products,0,4887,q);
		  mingchi = values[index];
		  printf("Fei tong hua ming chi %d\n",mingchi);
		  }

	  }
	  return mingchi;
}
vector<int> Player:: StatisticsCard(vector<string> card)
{

  vector<int> typeCard[4];//0:SPADES 1:HEARTS 2:CLUBS 3:DIAMONDS
  vector<int> digital;
  vector<int> cardtype;
  int digiNum[13] = { 0 };
  int shuzi;
  srand(time(NULL));
  for (int i = 0; i<m_iCardNum; i++)
  {
          shuzi = 0;
          shuzi = strConvertInt(card[i][1]);
          digiNum[shuzi - 2]++;
          switch (card[i][0])
          {
          case 'S':
                  typeCard[0].push_back(shuzi);
                  break;
          case 'H':
                  typeCard[1].push_back(shuzi);
                  break;
          case 'C':
                  typeCard[2].push_back(shuzi);
                  break;
          case 'D':
                  typeCard[3].push_back(shuzi);
                  break;
          default:
                  m_iError = 1;
          }
          digital.push_back(shuzi);
  }
  for (int i = 0; i<4; i++)
          sort(typeCard[i].begin(), typeCard[i].end());
  sort(digital.begin(), digital.end());
  if (2 == card.size())
    {
            cardtype.clear();
            if (digital[0] == digital[1]) {
                    cardtype.push_back(1);
                    cardtype.push_back(digital[0]);
                    cardtype.push_back(digital[1]);
            }
            else if (typeCard[0].size() == 2 || typeCard[1].size() == 2 || typeCard[2].size() == 2 || typeCard[3].size() == 2)
            {
                    cardtype.push_back(2);
                    cardtype.push_back(digital[0]);
                    cardtype.push_back(digital[1]);
            }
            else
            {
                    cardtype.push_back(3);
                    cardtype.push_back(digital[0]);
                    cardtype.push_back(digital[1]);
            }

    }
  else
    {
      cardtype.clear();
      for (int i = 0; i<5; i++)
      {
              cardtype.push_back(digital[i]);
      }
      for (int i = 0; i<4; i++)
      {
              cardtype.push_back(typeCard[i].size());
      }
      for (int i = 0; i<13; i++)
      {
              cardtype.push_back(digiNum[i]);
      }
    }
  return cardtype;
}
void Player::initCardType()
{

        // m_vCardType;//dui zi return 1, tong hua return 2, qi ta return 3
      if(m_iCardNum == 2)
        {
          m_vCardType = StatisticsCard(m_sCard);
        }
      else//5 6 7 first push the sort card and push the card's number of eatch of colour
      {
         m_vCardTypeSum.clear();
         vector<string> statisCard;
          if( 5 == m_iCardNum)
            {
                m_vCardTypeSum.push_back(StatisticsCard(m_sCard));
            }
          else if(6 == m_iCardNum)
            {

              for(int i=0 ; i<6 ;i++ )
                {
                  statisCard.clear();
                  for(int j=0 ; i<5;j++)
                    statisCard.push_back(m_sCard[sixcardsixselect[i][j]] );
                  m_vCardTypeSum.push_back(StatisticsCard(statisCard));
                }
            }
          else
            {
              for(int i=0 ; i<21 ;i++ )
                {
                  statisCard.clear();
                  for(int j=0 ; i<5;j++)
                    statisCard.push_back(m_sCard[sevencardtwentyoneselect[i][j]] );
                  m_vCardTypeSum.push_back(StatisticsCard(statisCard));
                }
            }
        }

	if(5 == m_iCardNum)
	  {
	    if(m_sCard.size() != 5)
	      {
		m_iWinRate = 0;
		m_iError = 2;
		printf("M-sCard error...............................................\n");
		return ;
	      }
	    int myranking = calculateRank(m_sCard);
	    vector<int> rankSt;
	    rankSt.clear();
	    rankSt.push_back(myranking);
	    vector<string> flop;
	    set<string> cardsearchtemp;
	    int r1,r2;
	    for(int i=0; i<100;i++)
	      {
		flop.clear();
		cardsearchtemp.clear();
		r1 = r2 =0;
		for(int j=0;j<3;j++)//add three flop card
		  {
		    flop.push_back(m_sCard[j+2]);
		  }
		for(int j=0;j<5;j++)
		  cardsearchtemp.insert(m_sCard[j]);

		for(int j=0;j<2;j++)//add two rand card
		  {
		    while (true) {
			r1 = rand()%4;
			r2 = rand()%13;
			if(cardsearchtemp.find(cardpool[r1][r2]) == cardsearchtemp.end())
			   break;
		      }
		    cardsearchtemp.insert(cardpool[r1][r2]);
		    flop.push_back(cardpool[r1][r2]);
		  }
		rankSt.push_back(calculateRank(flop));

	      }
	    sort(rankSt.begin(),rankSt.end());
	    int winmycardnum = 0;
	    for(;winmycardnum<rankSt.size();winmycardnum++)
	      {
		if(myranking == rankSt[winmycardnum])
		  break;
	      }
	    printf("myranking = %d %d will loss my card in 100 hand\n",myranking,100 - winmycardnum);
	    m_iWinRate = 100 - winmycardnum;
	  }
	else if(6 == m_iCardNum)
	  {

	    if(m_sCard.size() != 6)
	      {
		m_iWinRate = 0;
		m_iError = 2;
		printf("M-sCard error...............................................\n");
		return ;
	      }
	    vector<int> rankSt;
	    rankSt.clear();
	    vector<string> flop;
	    flop.clear();
	    int myranking =0;
	    for(int i=0;i<6;i++)
	      {
		flop.clear();
		for(int j=0;j<5;j++)
		  {
		    flop.push_back(m_sCard[sixcardsixselect[i][j]]);
		  }
		myranking = calculateRank(flop);
		rankSt.push_back(myranking);
	      }
	    sort(rankSt.begin(),rankSt.end());
	    myranking = rankSt[0];
	    rankSt.clear();
	    rankSt.push_back(myranking);
	    set<string> cardsearchtemp;
	    int r1,r2;

	    for(int i=0; i<5;i++)
	      {
		flop.clear();
		cardsearchtemp.clear();
		r1 = r2 =0;
		for(int j=0;j<4;j++)//add three flop card
		  {
		    flop.push_back(m_sCard[j+2]);

		  }
		for(int j=0;j<6;j++)
		  cardsearchtemp.insert(m_sCard[j]);

		for(int j=0;j<1;j++)//add one rand card
		  {
		    while (true) {
			r1 = rand()%4;
			r2 = rand()%13;
			if(cardsearchtemp.find(cardpool[r1][r2]) == cardsearchtemp.end())
			   break;
		      }
		    cardsearchtemp.insert(cardpool[r1][r2]);
		    flop.push_back(cardpool[r1][r2]);
		  }
		rankSt.push_back(calculateRank(flop));

	      }

	    for(int i=0; i<95;i++)
	      {
		flop.clear();
		cardsearchtemp.clear();
		vector<string> tempCard(m_sCard.begin()+2,m_sCard.end());
		r1 = rand()%4;
		tempCard.erase(tempCard.begin() + r1);
		r1 = r2 =0;
		for(int j=0;j<3;j++)//add three card from four Community Cards
		  {
		    flop.push_back(tempCard[j]);
		  }
		for(int j=0;j<6;j++)
		  cardsearchtemp.insert(m_sCard[j]);

		for(int j=0;j<2;j++)//add two rand card
		  {
		    while (true) {
			r1 = rand()%4;
			r2 = rand()%13;
			if(cardsearchtemp.find(cardpool[r1][r2]) == cardsearchtemp.end())
			  break;
		      }
		    cardsearchtemp.insert(cardpool[r1][r2]);
		    flop.push_back(cardpool[r1][r2]);
		  }
		rankSt.push_back(calculateRank(flop));

	      }
	    sort(rankSt.begin(),rankSt.end());
	    int winmycardnum = 0;
	    for(;winmycardnum<rankSt.size();winmycardnum++)
	      {
		if(myranking == rankSt[winmycardnum])
		  break;
	      }
	    printf("myranking = %d %d will loss my card in 100 hand\n",myranking,100 - winmycardnum);
	    m_iWinRate = 100 - winmycardnum;
	  }
	else if(m_iCardNum == 7)
	  {
	    if(m_sCard.size() != 7)
	      {
		m_iWinRate = 0;
		m_iError = 2;
		printf("M-sCard error...............................................\n");
		return ;
	      }
	    vector<int> rankSt;
	    rankSt.clear();
	    vector<string> flop;
	    flop.clear();
	    int myranking =0;
	    for(int i=0;i<21;i++)
	      {
		flop.clear();
		for(int j=0;j<5;j++)
		  {
		    flop.push_back(m_sCard[sevencardtwentyoneselect[i][j]]);
		  }
		myranking = calculateRank(flop);
		rankSt.push_back(myranking);
	      }
	    sort(rankSt.begin(),rankSt.end());
	    myranking = rankSt[0];
	    rankSt.clear();
	    rankSt.push_back(myranking);
	    set<string> cardsearchtemp;
	    int r1,r2;

	    for(int i=0; i<16;i++)
	      {
		flop.clear();
		cardsearchtemp.clear();
		vector<string> tempCard(m_sCard.begin()+2,m_sCard.end());
		r1 = rand()%5;
		tempCard.erase(tempCard.begin() + r1);
		r1 = r2 =0;
		for(int j=0;j<4;j++)//add four card from five Community Cards
		  {
		    flop.push_back(tempCard[j]);
		  }
		for(int j=0;j<7;j++)
		  cardsearchtemp.insert(m_sCard[j]);

		for(int j=0;j<1;j++)//add one rand card
		  {
		    while (true) {
			r1 = rand()%4;
			r2 = rand()%13;
			if(cardsearchtemp.find(cardpool[r1][r2]) == cardsearchtemp.end())
			break;
		      }
		    cardsearchtemp.insert(cardpool[r1][r2]);
		    flop.push_back(cardpool[r1][r2]);
		  }
		rankSt.push_back(calculateRank(flop));

	      }

	    for(int i=0; i<84;i++)
	      {
		flop.clear();
		cardsearchtemp.clear();
		vector<string> tempCard(m_sCard.begin()+2,m_sCard.end());
		r1 = rand()%5;
		tempCard.erase(tempCard.begin() + r1);
		r1 = rand()%4;
		tempCard.erase(tempCard.begin() + r1);
		r1 = r2 =0;
		for(int j=0;j<3;j++)//add three card from four Community Cards
		  {
		    flop.push_back(tempCard[j]);
		  }
		for(int j=0;j<7;j++)
		  cardsearchtemp.insert(m_sCard[j]);
		for(int j=0;j<2;j++)//add two rand card
		  {
		    while (true) {
			r1 = rand()%4;
			r2 = rand()%13;
			if(cardsearchtemp.find(cardpool[r1][r2]) == cardsearchtemp.end())
			break;
		      }
		    cardsearchtemp.insert(cardpool[r1][r2]);
		    flop.push_back(cardpool[r1][r2]);
		  }
		rankSt.push_back(calculateRank(flop));

	      }
	    sort(rankSt.begin(),rankSt.end());
	    int winmycardnum = 0;
	    for(;winmycardnum<rankSt.size();winmycardnum++)
	      {
		if(myranking == rankSt[winmycardnum])
		  break;
	      }
	    printf("myranking = %d %d will loss my card in 100 hand\n",myranking,100 - winmycardnum);
	    m_iWinRate = 100 - winmycardnum;
	  }
	else
	  {
	    m_iWinRate = 0;
	    m_iError = 2;
	    printf("M-sCard error...............................................\n");
	  }



	//  for(int i=0; i<m_vCardType.size() ;i++)
	//    printf("%d ",m_vCardType[i]);
	//  printf("\n");
	//  fflush(stdout);
}

vector<int> Player::getMaxCardType()
{
	return m_vCardType;
}

void Player::handAllInPlayer(inquireInfo &inqInfo)
{
  if(inqInfo.allinNum == 0) return ;
  vector<int>::iterator be = inqInfo.allInPlayerBet.begin();
  for(;be != inqInfo.allInPlayerBet.end() ;++be)
    {
      if(*be < m_iFrontHaveBet)
        inqInfo.callNum++;
      else
        inqInfo.raiseNum++;
    }
}
inquireInfo Player::inquire(char *info)
{
	m_strNofoldPlayID.clear();
	char *findbegin, *findend, *findtemp;
	findbegin = info;
	vector<string > vData;
	string s = "";
	findtemp = strchr(findbegin, '\n');
	findbegin = findtemp + 1;

	bool bCouldNotify = true;
	if(strstr(findbegin,"blind") != NULL)
	  bCouldNotify = false;
	inquireInfo inqInfo;
	memset(&inqInfo, '\0', sizeof(inqInfo));
	inqInfo.noFlopNum = inqInfo.checkNum = inqInfo.callNum = inqInfo.raiseNum = inqInfo.allinNum = inqInfo.noFlodLeastJetton = 0;
	inqInfo.allInPlayerBet.clear();
	bool bLastPlayBeginRaise = false;
	int otherSumBet = 0;

	m_iFrontHaveBet = 0;
	try
	{
		while (*findbegin != 't')
		{
			s = "";
			vData.clear();
			findend = strchr(findbegin, '\n');
			*findend = '\0';

			slipInfo(findbegin, vData);
			*findend = '\n';
			findbegin = findend + 1;
			if(vData[4] == "raise" && bLastPlayBeginRaise == false)
			  {
			    bLastPlayBeginRaise = true;
			    m_bLastPlayerBeginRaise = true;
			    m_strFrontRaisePlayerID = vData[0];
			  }
			if(bLastPlayBeginRaise && vData[4] != "fold")
			  {
			    m_bLastPlayerBeginRaise = false;
			  }
			if (vData[4] != "fold")
			{
				printf("inquire:jetton=%s\n", vData[1].c_str());
				int jetton = atoi(vData[1].c_str());
				inqInfo.noFlodLeastJetton = inqInfo.noFlodLeastJetton > jetton ? jetton : inqInfo.noFlodLeastJetton;
				m_strNofoldPlayID.push_back(vData[0]);

				m_iFrontHaveBet = max(m_iFrontHaveBet,atoi(vData[3].c_str()));//set max bet front of me
			}
			if (vData[4] == "check")
				++inqInfo.checkNum;
			else if (vData[4] == "call")
				++inqInfo.callNum;
			else if (vData[4] == "raise")
				++inqInfo.raiseNum;
			else if (vData[4] == "all_in")
			  {
			    ++inqInfo.allinNum;
			    inqInfo.allInPlayerBet.push_back(atoi(vData[3].c_str()));
			  }

			else if (vData[4] == "blind")
				++inqInfo.blindNum;
			else if (vData[4] == "fold")
				++inqInfo.foldNum;

			if(bCouldNotify && !m_bHaveNotify && vData[4] == "fold"  )
			  {
			    ++ m_mFoldNum[vData[0]];
			    m_bHaveNotify = true;
			  }

			otherSumBet += atoi(vData[3].c_str());

		}
		findend = strchr(findbegin, '\n');
		*findend = '\0';
		printf("inqire:totalBet=%s\n", findbegin + strlen("total pot: "));
		m_iTotalBet = atoi(findbegin + strlen("total pot: "));

		*findend = '\n';

		int myHaveBet = m_iTotalBet - otherSumBet;
		setHaveBet(myHaveBet);

		inqInfo.noFlopNum = m_iPlayerNum - inqInfo.foldNum;
	}
	catch (...)
	{
		m_iError = 2;
	}
	handAllInPlayer(inqInfo);
	return inqInfo;

}
void Player::notify(char *info)
{
  char *findbegin, *findend, *findtemp;
  findbegin = info;

  if(m_bHaveNotify ) return ;
  if(strstr(findbegin,"blind") != NULL) return ;

  vector<string > vData;
  string s = "";
  findtemp = strchr(findbegin, '\n');
  findbegin = findtemp + 1;
  try
  {
          while (*findbegin != 't')
          {
                  s = "";
                  vData.clear();
                  findend = strchr(findbegin, '\n');
                  *findend = '\0';

		  slipInfo(findbegin, vData);
		  *findend = '\n';
		  findbegin = findend + 1;
		  if(vData[4] == "fold")
		   ++ m_mFoldNum[vData[0]];

	  }
	  findend = strchr(findbegin, '\n');
	  *findend = '\0';
	  *findend = '\n';
	  m_bHaveNotify = true;
  }
  catch (...)
  {
          m_iError = 2;
  }
}

bool sockConnect(int &sockfd, struct sockaddr_in *sockaddr, int &len)
{
	//  int result = 0;
	//  for(int i=0 ; i<3 ; i++)
	//    {
	//      result = connect(sockfd, (struct sockaddr *)sockaddr, len);
	//      if( 0 != result)
	//        {
	//          printf("connect error \n");
	//          if(i==3)
	//            return false;
	//          sleep(1);
	//        }
	//      else break;
	//    }
	while (connect(sockfd, (struct sockaddr *)sockaddr, len) != 0) sleep(1);
	return true;
}







class ActionClass
{
public:
	ActionClass(Player &play);
private:
public:
	int iPlayNum;
	int mypos;
	int leastBet;
	int myRemJetton;
	int raiseLeastBet;
	int myMoney;
	int totalBet;
	int lunJiShu;
	int jettonSum;
	int haveBet;
	string actionHead[5];
};
ActionClass::ActionClass(Player &play)
{
	iPlayNum = play.getPlayNum();
	haveBet = play.getHaveBet();
	mypos = play.getSeatPos();
	leastBet = play.getLeastBet();
	myRemJetton = play.getMyRemJetton();
	myMoney = play.getMyMoney();
	raiseLeastBet = play.getRaiseLeast();
	totalBet = play.getTotalBet();
	lunJiShu = play.getLunJiShu();
	jettonSum = play.getSumJetton();
	string actionHead11[5] = { " check ", " call ", " raise ", " all_in ", " fold " };
	for (int i = 0; i<5; i++)
	{
		actionHead[i] = actionHead11[i];

	}
}

string commonAction(Player &play, inquireInfo &inqInfo, vector<int> &vCardType, ActionClass &action_Int)
{
	if (inqInfo.foldNum == action_Int.iPlayNum - 1)
		return action_Int.actionHead[0];
//    if((action_Int.myRemJetton-(MAXJU-action_Int.lunJiShu)/action_Int.iPlayNum*action_Int.raiseLeastBet/2*3 )>= action_Int.jettonSum/2)
//        return action_Int.actionHead[4];
	return "";
}

string stealpool(inquireInfo & inqInfo, ActionClass &action_Int)
{
  lunSum++;
  if(lunSum <=CHECKMAX_INTEL && action_Int.leastBet < action_Int.totalBet )
    return action_Int.actionHead[1];
  else
    return action_Int.actionHead[4];
}
int pos[][8] =
{
    /* 0 1 2 3 4 5 6 7 *//* 1 qian 2 zhong 3 hou 4 small blind 5 big blind*/
/*2*/ {3,4,0,0,0,0,0,0},
/*3*/ {3,4,5,0,0,0,0,0},
/*4*/ {3,4,5,2,0,0,0,0},
/*5*/ {3,4,5,1,2,0,0,0},
/*6*/ {3,4,5,1,1,2,0,0},
/*7*/ {3,4,5,1,1,2,2,0},
/*8*/ {3,4,5,1,1,2,2,3}
};
int twoCardLevel[][91] = {
     /*2A 3A 4A 5A 6A 7A 8A 9A 1A JA QA KA AA 2K 3K 4K 5K 6K 7K 8K 9K 1K JK QK KK 2Q 3Q 4Q 5Q 6Q 7Q 8Q 9Q 1Q JQ QQ 2J 3J 4J 5J 6J 7J 8J 9J 1J JJ 21 31 41 51 61 71 81 91 11 29 39 49 59 69 79 89 99 28 38 48 58 68 78 88 27 37 47 57 67 77 26 36 46 56 66 25 35 45 55 24 34 44 23 33 22 */
 /*o*/{ 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 5, 6, 6, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 6, 1, 1, 1, 1, 1, 1, 1, 1, 2, 5, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 3, 1, 3, 3 },
 /*s*/{ 2, 2, 2, 2, 2, 2, 2, 2, 4, 5, 5, 6, 0, 1, 1, 1, 1, 1, 1, 1, 2, 3, 3, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }

};

int playerActionLevel(inquireInfo &inqInfo)
{
  if( inqInfo.callNum == 0  && inqInfo.raiseNum == 0)
    return 6;
  else if( inqInfo.callNum == 1  && inqInfo.raiseNum == 0)
    return 5;
  else if( inqInfo.callNum >= 2  && inqInfo.raiseNum == 0)
    return 4;
  else if(inqInfo.raiseNum == 1 && inqInfo.callNum == 0)
    return 3;
  else if(inqInfo.raiseNum == 1 && inqInfo.callNum >= 1)
    return 2;
  else
    return 1;
}
string CardRaise(int bet,ActionClass &action_Int)
{
  char cLeastBet[32]={'\0'};
  string strLeastBet = "";
  string strActionResult = "";
  sprintf(cLeastBet, "%d ",  bet);
  strLeastBet = cLeastBet;
  strActionResult = strActionResult + action_Int.actionHead[2] + strLeastBet;
  return strActionResult;
}

string twocardaction(Player &play,inquireInfo &inqInfo,ActionClass &action_Int,vector<int> &vCardType)
{
	int row = 0;
	for (int i = 13; i>vCardType[2] - 1; i--)
		row += i;
	row += vCardType[1] - 2;
	int icardlevel = 0;
	if(vCardType[0] == 2)
	  icardlevel = twoCardLevel[1][row];
	else
	  icardlevel = twoCardLevel[0][row];
	int imypos = pos[action_Int.iPlayNum-2][action_Int.mypos];
	int iplayLevel = playerActionLevel(inqInfo);

	string returnAction = "";
	if(play.getFirstInqurieStatus())
	  {
	    play.setFirstInquireFalse();

	      switch(icardlevel)
		{
		case 6://raise
		  return CardRaise(action_Int.raiseLeastBet*6,action_Int);//raise
		  break;
		case 5:
		  {
		    if(iplayLevel >= 4)
		      {
		      int needbet = action_Int.raiseLeastBet*(4+inqInfo.callNum);
		      returnAction = CardRaise(needbet,action_Int);
		      }

                    else if(iplayLevel == 3)
                     {
                      if(imypos == 1)
                      {
                          if(action_Int.leastBet >= 2* action_Int.raiseLeastBet)
                              returnAction = action_Int.actionHead[4];//fold
                          else
                              returnAction = action_Int.actionHead[1];//call
                      }

                      else
                      {
                      int needbet = action_Int.leastBet*(3+inqInfo.callNum);
                      returnAction = CardRaise(needbet,action_Int);//raise
                      }

                    }
                    else if(iplayLevel == 2)
                      returnAction = action_Int.actionHead[1];//call
                    else
                       returnAction = action_Int.actionHead[4];
                     break;
                  }

                case 4:
                  {
                    if(iplayLevel >= 4)
                  {
                    if(imypos == 1)
                      returnAction = action_Int.actionHead[4];//fold
                    else
                      returnAction = CardRaise(action_Int.raiseLeastBet,action_Int);//raise
                  }

                    else if(iplayLevel == 3)
                    {
                    if(imypos != 5)
                      returnAction = action_Int.actionHead[4];//fold
                    else
                      returnAction = action_Int.actionHead[1];//call
                    }
                    else if(iplayLevel == 2)
                     {
                    if(imypos != 5 &&vCardType[0]== 2 && vCardType[1] == 12 )
                      returnAction = action_Int.actionHead[1];//call
                    else if(imypos != 5)
                      returnAction = action_Int.actionHead[4];//fold
                    else
                      returnAction = action_Int.actionHead[1];//call
                     }

                    else
                     returnAction = action_Int.actionHead[4];
                     break;
                  }
                case 3:
                  {
                    if(iplayLevel == 6)
                   {
                    if(imypos == 1 || imypos == 2)
                      returnAction = action_Int.actionHead[4];//fold
                    else
                      returnAction = CardRaise(action_Int.raiseLeastBet,action_Int);
                   }
                    else if(iplayLevel == 5)
                    {
                    if(imypos == 1 || imypos ==2)
                      returnAction = action_Int.actionHead[4];//fold
                    else if(imypos == 3 || imypos ==4)
                      returnAction = action_Int.actionHead[1];//call
                    else
                      returnAction = action_Int.actionHead[0];//check

                   }
                    else if(iplayLevel == 4)
                     {
                    if(imypos != 5)
                      returnAction = action_Int.actionHead[1];//call
                    else
                      returnAction = action_Int.actionHead[0];//check
                      }
                    else if(iplayLevel == 3)
                      {
                    if(imypos == 1 || imypos ==2)
                      returnAction = action_Int.actionHead[4];//fold
                    else
                      returnAction = action_Int.actionHead[1];//call
                       }
                    else if(iplayLevel == 2)
                     {
                          returnAction = action_Int.actionHead[1];//call
                     }
                    else
                      {
                         returnAction = action_Int.actionHead[4];//fold
                      }
                     break;
                  }

                case 2:
                  {
                    if(iplayLevel == 6)
                  {
                        if(imypos == 1 || imypos == 2)
                          returnAction = action_Int.actionHead[4];
                        else
                          returnAction = CardRaise(action_Int.raiseLeastBet,action_Int);
                  }
                    else if(iplayLevel == 5)
                  {
                        if(imypos <= 3)
                          returnAction = action_Int.actionHead[4];
                        else if(imypos == 4)
                          returnAction = action_Int.actionHead[1];
                        else
                          returnAction = action_Int.actionHead[0];
                  }
                    else if(iplayLevel == 4)
                  {
                        if(imypos <= 2)
                          returnAction = action_Int.actionHead[4];
                        else if(imypos <= 4)
                          returnAction = action_Int.actionHead[1];
                        else
                          returnAction = action_Int.actionHead[0];
                  }
                    else
                  {
                        returnAction = action_Int.actionHead[4];
                  }
                     break;
                  }
                case 1:
                  {
                   if(iplayLevel == 6 && imypos >=4)
                  returnAction = stealpool(inqInfo,action_Int);
                    else
                  returnAction = action_Int.actionHead[4];
                     break;
                  }
                default:
                  printf("card level error ...........................................\n");
                  break;
                }
          }
        else
          {

            switch(icardlevel)
              {
              case 6://raise
                {
                  if(lunSum < TWOCARDMAXRAISENUM)
                    return CardRaise(action_Int.totalBet/2,action_Int);//raise
                  else
                    {
                      if(action_Int.myRemJetton >= 4* action_Int.haveBet)
                        returnAction = action_Int.actionHead[1];
                      else
                        returnAction = action_Int.actionHead[3];
                    }

                  break;
                }
              case 5:
                {

                if(action_Int.myRemJetton < 4 * action_Int.haveBet)
                  returnAction = action_Int.actionHead[3];
                else
                  returnAction = action_Int.actionHead[4];
                 break;
                }

              case 4:
                {
                if(action_Int.myRemJetton < 2.5 * action_Int.haveBet)
                  returnAction = action_Int.actionHead[3];
                else
                  returnAction = action_Int.actionHead[4];
                break;

                }
              case 3:
                returnAction = action_Int.actionHead[4];
                 break;
              case 2:
                returnAction = action_Int.actionHead[4];
                 break;
              case 1:
                  returnAction = action_Int.actionHead[4];
                   break;
              default:
                printf("card level error ...........................................\n");
                break;
              }
          }
     return returnAction ;

}
string fiveCardAction(Player &play,int icardlevel,inquireInfo &inqInfo, ActionClass &action_Int,int winrate)
{
  string  returnAction = "" ;
  int iplayLevel = playerActionLevel(inqInfo);
  int imypos = pos[action_Int.iPlayNum-2][action_Int.mypos];
  if(play.getFirstInqurieStatus())
    {
      play.setFirstInquireFalse();

	switch(icardlevel)
	  {
	  case 6://raise
	    return CardRaise(action_Int.totalBet/2,action_Int);//raise
	    break;
	  case 5:
	    {
	      if(iplayLevel >= 4)
		{
		int needbet = action_Int.raiseLeastBet*(4+inqInfo.callNum);
		returnAction = CardRaise(needbet,action_Int);
		}

              else if(iplayLevel == 3)
               {
                if(imypos == 1)
                {
                    if(action_Int.leastBet >= 2* action_Int.raiseLeastBet)
                        returnAction = action_Int.actionHead[4];//fold
                    else
                        returnAction = action_Int.actionHead[1];//call
                }

                else
                {
                int needbet = action_Int.leastBet*(3+inqInfo.callNum);
                returnAction = CardRaise(needbet,action_Int);//raise
                }

              }
              else if(iplayLevel == 2)
                returnAction = action_Int.actionHead[1];//call
              else
                 returnAction = action_Int.actionHead[4];
               break;
            }

          case 4:
            {
              if(iplayLevel >= 4)
            {
              if(imypos == 1)
                returnAction = action_Int.actionHead[4];//fold
              else
                returnAction = CardRaise(action_Int.raiseLeastBet,action_Int);//raise
            }

              else if(iplayLevel == 3)
              {
              if(imypos != 5)
                returnAction = action_Int.actionHead[4];//fold
              else
                returnAction = action_Int.actionHead[1];//call
              }
              else if(iplayLevel == 2)
              {
              if(imypos != 5)
                returnAction = action_Int.actionHead[4];//fold
              else
                returnAction = action_Int.actionHead[1];//call
               }

              else
               returnAction = action_Int.actionHead[4];
               break;
            }
          case 3:
            {
              if(iplayLevel == 6)
             {
              if(imypos == 1 || imypos == 2)
                returnAction = action_Int.actionHead[4];//fold
              else
                returnAction = CardRaise(action_Int.raiseLeastBet,action_Int);
             }
              else if(iplayLevel == 5)
              {
              if(imypos == 1 || imypos ==2)
                returnAction = action_Int.actionHead[4];//fold
              else if(imypos == 3 || imypos ==4)
                returnAction = action_Int.actionHead[1];//call
              else
                returnAction = action_Int.actionHead[0];//check

             }
              else if(iplayLevel == 4)
               {
              if(imypos != 5)
                returnAction = action_Int.actionHead[1];//call
              else
                returnAction = action_Int.actionHead[0];//check
                }
              else if(iplayLevel == 3)
                {
              if(imypos == 1 || imypos ==2)
                returnAction = action_Int.actionHead[4];//fold
              else
                returnAction = action_Int.actionHead[1];//call
                 }
              else if(iplayLevel == 2)
               {
                    returnAction = action_Int.actionHead[1];//call
               }
              else
                {
                   returnAction = action_Int.actionHead[4];//fold
                }
               break;
            }

          case 2:
            {
              if(iplayLevel == 6)
            {
                  if(imypos == 1 || imypos == 2)
                    returnAction = action_Int.actionHead[4];
                  else
                    returnAction = CardRaise(action_Int.raiseLeastBet,action_Int);
            }
              else if(iplayLevel == 5)
            {
                  if(imypos <= 3)
                    returnAction = action_Int.actionHead[4];
                  else if(imypos == 4)
                    returnAction = action_Int.actionHead[1];
                  else
                    returnAction = action_Int.actionHead[0];
            }
              else if(iplayLevel == 4)
            {
                  if(imypos <= 2)
                    returnAction = action_Int.actionHead[4];
                  else if(imypos <= 4)
                    returnAction = action_Int.actionHead[1];
                  else
                    returnAction = action_Int.actionHead[0];
            }
              else
            {
                  returnAction = action_Int.actionHead[4];
            }
               break;
            }
          case 1:
            {
             if(iplayLevel == 6 && imypos >=4)
            returnAction = stealpool(inqInfo,action_Int);
              else
            returnAction = action_Int.actionHead[4];
               break;
            }
          default:
            printf("card level error ...........................................\n");
            break;
          }
    }
  else
    {

      switch(icardlevel)
        {
        case 6://raise
          {
            if(lunSum < winrate)
              return CardRaise(action_Int.totalBet/2,action_Int);//raise
            else
              {
                if(action_Int.myRemJetton >= 4* action_Int.haveBet)
                  returnAction = action_Int.actionHead[1];
                else
                  returnAction = action_Int.actionHead[3];
              }

            break;
          }
        case 5:
          {
            if(action_Int.raiseLeastBet <= 1  && action_Int.totalBet >2*action_Int.leastBet)
                returnAction = action_Int.actionHead[1];
            else
              {
                if(action_Int.myRemJetton < 4 * action_Int.haveBet)
                    returnAction = action_Int.actionHead[3];
                 else
                    returnAction = action_Int.actionHead[4];
              }
           break;
          }

        case 4:
          {
            if(action_Int.raiseLeastBet <= 1 && action_Int.leastBet < action_Int.myRemJetton/10 && action_Int.totalBet >2 *action_Int.leastBet)
                returnAction = action_Int.actionHead[1];
          else if(action_Int.myRemJetton < 2.5 * action_Int.haveBet)
            returnAction = action_Int.actionHead[3];
          else
            returnAction = action_Int.actionHead[4];
          break;

          }
        case 3:
        case 2:
        case 1:
            returnAction = action_Int.actionHead[4];
             break;
        default:
          printf("card level error ...........................................\n");
          break;
        }
    }
return returnAction ;

}

int cardPotential(Player &play,int icardlevel,inquireInfo &inqInfo, ActionClass &action_Int)
{
    vector<int> vCardType = play.getMaxCardType();
    int iCardNum = play.getCardNum();
    int iPotential = 0;
    if(iCardNum  == 5)
      {
         if(vCardType[iCardNum+0] == 4 || vCardType[iCardNum+1] == 4 ||vCardType[iCardNum+2] == 4 || vCardType[iCardNum+3] == 4)//ting tong hua
           {
             if((vCardType[iCardNum-2] - vCardType[0] == 3) || (vCardType[iCardNum-1] - vCardType[1] == 3))//liang duan ting shun zi + tong hua
               iPotential = 15;
            if(vCardType)
           }
      }

}
string action(Player &play, inquireInfo & inqInfo)
{

	//  if(inqInfo.raiseNum ==0 && inqInfo.checkNum == 0 && inqInfo.callNum ==0 && inqInfo.foldNum == 0 && inqInfo.blindNum !=0)
	//    return fastHandle(play);



	ActionClass action_Int(play);
	int iCardNum = play.getCardNum();
	vector<int> vCardType = play.getMaxCardType();

    string strCom = commonAction(play, inqInfo, vCardType, action_Int);
        if (strCom != "")
                return strCom;
        // have solved all_in player
        if(iCardNum == 2)
          {
            return twocardaction(play,inqInfo,action_Int,vCardType);

	  }
	else
	  {
	    int winRate = play.getWinRate();
	    int cardlevel = 0;
	    int jichu = 90;
	    int iremindlevel = play.getRemainderLevel();
	    if(iremindlevel < 0) jichu = 85;
	    if(iremindlevel > 0) jichu = 95;
	    if(winRate >= jichu) cardlevel = 6;
	    else if(winRate >= jichu -10) cardlevel = 5;
	    else if(winRate >= jichu -20) cardlevel = 4;
	    else if(winRate >= jichu -30) cardlevel = 3;
	    else if(winRate >= jichu -40) cardlevel = 2;
	    else cardlevel = 1;
	    if(iCardNum == 5)
	      {
		return fiveCardAction(play,cardlevel,inqInfo,action_Int,winRate/10-5);
	      }
	    else if(iCardNum == 6)
	      {
		return fiveCardAction(play,cardlevel,inqInfo,action_Int,winRate/10-5);
	      }
	    else if(iCardNum == 7)
	      {
		return fiveCardAction(play,cardlevel,inqInfo,action_Int,winRate/10-5);
	      }
	    else
	      {
		printf("cardnum error..............................................\n");
	      }
	  }
}

#define MAXREAD 2048
void pot_win(Player &play, char *argv);
int main(int argc, char *argv[])
{
	if (argc != 6){
		printf("argc error\n");
		return -1;
	}
	int sockfd;
	struct sockaddr_in address, addmy;
	memset(&address, 0, sizeof(address));
	memset(&addmy, 0, sizeof(addmy));
	int result;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	addmy.sin_family = AF_INET;
	addmy.sin_addr.s_addr = inet_addr(argv[3]);
	addmy.sin_port = htons((int)(atoi(argv[4])));


	if (-1 == bind(sockfd, (struct sockaddr *)&addmy, sizeof(addmy)))
	{
		printf("Error : addmy bind failed!\n");
		return -1;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(argv[1]);
	address.sin_port = htons((int)(atoi(argv[2])));

	int len = sizeof(address);
	if (!sockConnect(sockfd, &address, len)) return -1;

	printf("success connect IP:%s\n", argv[3]);
	char tempbuffer[MAXREAD];
	//  char regbuffer[MAXREAD];
	//char readbuffer[MAXREAD];
	memset(tempbuffer, '\0', MAXREAD);
	//  memset(regbuffer,'\0',MAXREAD);
	//memset(readbuffer,'\0',MAXREAD);

	strcpy(tempbuffer, "reg: ");
	strcat(tempbuffer, argv[5]);
	strcat(tempbuffer, " ");
	strcat(tempbuffer, argv[5]);
	strcat(tempbuffer, " need_notify ");

	while (0 >= send(sockfd, tempbuffer, strlen(tempbuffer) + 1, 0)) sleep(1);


	printf("reg success!\n");
	Player play(argv[5]);

	pthread_t a_thread;
	DataBufferPool<char > databuf1(10, MAXREAD);
	DataStruct *threadData = new DataStruct;
	threadData->socknum = sockfd;
	threadData->pSome = (void*)& databuf1;
	int res = pthread_create(&a_thread, NULL, thread_function, (void *)threadData);
	if (res != 0)
		printf("thread create error\n");
	int llll = 0;
	bool bLianPai = false;
	char *tmp = NULL;
	while (1)
	{
		bool bOver = false;
		bool bBenJuOver = false;
		string s;
		play.init();
		char *pp = NULL;
		while (1)
		{

			if (!bLianPai)
			{
				memset(tempbuffer, '\0', MAXREAD);
				databuf1.ReadData((void*)tempbuffer, MAXREAD);
				tmp = tempbuffer;
			}


			char *findpos = NULL;
			if ((findpos = strstr(tmp, "seat/")) != NULL)
			{
				*findpos = '\0';

				if ((pp = strstr(tmp, "pot-win/")) != NULL)
				{
					printf("%d pot-win\n", llll);
					//		            pot_win(play,argv[5]);
					bLianPai = true;
					*findpos = 's';
					tmp = findpos;
					continue;

				}
				else
				{
					*findpos = 's';
					llll = play.getLunJiShu();
					printf("%d seat\n", llll);
					pp = strstr(tmp, "/seat");
					int len = strlen("/seat");
					char ct = *(pp + len);
					*(pp + len) = '\0';
					play.setSeat(findpos);
					*(pp + len) = ct;
					bLianPai = false;
				}





			}
			if ((findpos = strstr(tmp, "blind/")) != NULL)
			{
				printf("%d blind\n", llll);

				pp = strstr(tmp, "/blind");
				int len = strlen("/blind");
				char ct = *(pp + len);
				*(pp + len) = '\0';
				play.blind(findpos);
				*(pp + len) = ct;

			}
			if ((findpos = strstr(tmp, "hold/")) != NULL)
			{
				printf("%d hold\n", llll);

				pp = strstr(tmp, "/hold");
				int len = strlen("/hold");
				char ct = *(pp + len);
				*(pp + len) = '\0';
				play.addCard(findpos);
				*(pp + len) = ct;
				lunSum = 0;

			}

			if ((findpos = strstr(tmp, "flop/")) != NULL)
			{
				printf("%d flop\n", llll);

				pp = strstr(tmp, "/flop");
				int len = strlen("/flop");
				char ct = *(pp + len);
				*(pp + len) = '\0';
				play.addCard(findpos);
				*(pp + len) = ct;
				lunSum = 0;

			}
			if ((findpos = strstr(tmp, "turn/")) != NULL)
			{
				printf("%d turn\n", llll);

				pp = strstr(tmp, "/turn");
				int len = strlen("/turn");
				char ct = *(pp + len);
				*(pp + len) = '\0';
				play.addCard(findpos);
				*(pp + len) = ct;
				lunSum = 0;

			}
			if ((findpos = strstr(tmp, "river/")) != NULL)
			{
				printf("%d river\n", llll);

				pp = strstr(tmp, "/river");
				int len = strlen("/river");
				char ct = *(pp + len);
				*(pp + len) = '\0';
				play.addCard(findpos);
				*(pp + len) = ct;
				lunSum = 0;

			}
			if ((findpos = strstr(tmp, "inquire/")) != NULL)
			{
				printf("%d inquire\n", llll);
				inquireInfo inq;
				memset(&inq, '\0', sizeof(inq));

				pp = strstr(tmp, "/inquire");
				int len = strlen("/inquire");
				char ct = *(pp + len);
				*(pp + len) = '\0';
				inq = play.inquire(findpos);
				*(pp + len) = ct;

				if (!play.getCardStatus() && play.getErrorNum() == 0)
				{
					s = action(play, inq);
				}
				else
				{
					if (play.gHisPlayNum == inq.foldNum + 1)
						s = " check ";
					else
						s = " fold ";
				}

				if (s.size() == 0)
					printf("action error in inqire!!\n");
				else
					printf("action=%s\n", s.c_str());

				send(sockfd, s.c_str(), s.size() + 1, 0);

			}
			if ((findpos = strstr(tmp, "showdown/")) != NULL)
			{
				printf("%d showdown\n", llll);
				pp = strstr(tmp, "/showdown");
				int len = strlen("/showdown");
				char ct = *(pp + len);
				*(pp + len) = '\0';
				//do sth
				*(pp + len) = ct;
			}
			if ((findpos = strstr(tmp, "pot-win/")) != NULL)
			{
				printf("%d pot-win\n", llll);

				pp = strstr(tmp, "/pot-win");
				int len = strlen("/pot-win");
				char ct = *(pp + len);
				*(pp + len) = '\0';
				//do sth
				*(pp + len) = ct;

				//		      pot_win(play,argv[5]);
				bBenJuOver = true;
			}
			if ((findpos = strstr(tmp, "notify/")) != NULL)
			{
				printf("%d notify\n", llll);

				pp = strstr(tmp, "/notify");
				int len = strlen("/notify");
				char ct = *(pp + len);
				*(pp + len) = '\0';
				//do sth
				play.notify(findpos);
				*(pp + len) = ct;
			}
			if ((findpos = strstr(tmp, "game-over")) != NULL)
			{
				printf("%d game-over\n", llll);
				bBenJuOver = true;
				bOver = true;
			}
			if ((findpos = strstr(tmp, "threadexit")) != NULL)
			{
				printf("%d game-over\n", llll);
				bBenJuOver = true;
				bOver = true;
			}

			if (bBenJuOver) break;

		}

		if (bOver)
		{
			printf("game over !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			break;
		}

	}

	close(sockfd);
	return 0;
}

void *thread_function(void *arg)
{
	DataStruct *threadData = (DataStruct*)arg;
	int sockfd = threadData->socknum;
	DataBufferPool<char> *bufferPool = (DataBufferPool<char >*)(threadData->pSome);
	int result;
	char tempbuffer[MAXREAD];
	int jishu = 0;
	while (1)
	{
		memset(tempbuffer, '\0', MAXREAD);
		result = recv(sockfd, tempbuffer, MAXREAD, 0);
		if (result > 0)
		{
			bufferPool->WriteData((void*)tempbuffer, MAXREAD);
		}
		else if (result == 0)
		{
			printf("sock close success!\n");
			bOver = true;
		}
		else
		{
			if (jishu == 3)
			{
				printf("read fail\n");
				bOver = true;
			}
			else{
				jishu++;
				continue;
			}


		}


		if (bOver)
			break;

	}
	char pexit[] = "thread exit!\n";
	sprintf(tempbuffer, "%s", "threadexit");
	bufferPool->WriteData((void*)tempbuffer, MAXREAD);
	pthread_exit(pexit);

}


void pot_win(Player &play, char *argv)
{

}
