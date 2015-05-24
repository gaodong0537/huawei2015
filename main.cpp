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
#include <pthread.h>
#include <algorithm>
#include <semaphore.h>
#include <cctype>
using namespace std;
int twoCardAction[][91] = {
		/*2A 3A 4A 5A 6A 7A 8A 9A 1A JA QA KA AA 2K 3K 4K 5K 6K 7K 8K 9K 1K JK QK KK 2Q 3Q 4Q 5Q 6Q 7Q 8Q 9Q 1Q JQ QQ 2J 3J 4J 5J 6J 7J 8J 9J 1J JJ 21 31 41 51 61 71 81 91 11 29 39 49 59 69 79 89 99 28 38 48 58 68 78 88 27 37 47 57 67 77 26 36 46 56 66 25 35 45 55 24 34 44 23 33 22 */
	/*8ren */{ 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 4, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	/*4ren */{ 1, 1, 1, 1, 1, 1, 2, 2, 3, 3, 4, 5, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1 },
	/*2ren */{ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 6, 6, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 5, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 6, 1, 1, 1, 1, 1, 1, 1, 1, 2, 5, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 3, 1, 1, 1, 3, 1, 1, 3, 1, 3, 3 },
	/*8tong*/{ 1, 1, 1, 1, 1, 1, 2, 2, 4, 4, 5, 5, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 0, 1, 1, 1, 1, 1, 1, 1, 1, 3, 0, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0 },
	/*4tong*/{ 2, 2, 2, 2, 2, 2, 3, 3, 3, 4, 6, 6, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 5, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 0, 1, 1, 1, 1, 1, 1, 1, 3, 0, 1, 1, 1, 1, 1, 1, 2, 0, 1, 1, 1, 1, 1, 2, 3, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 0, 1, 2, 2 },
	/*2tong*/{ 3, 3, 3, 3, 3, 4, 4, 4, 5, 5, 6, 7, 0, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 6, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5, 0, 1, 1, 1, 1, 1, 1, 1, 2, 4, 0, 1, 1, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 3, 0, 1, 1, 1, 1, 1, 2, 4, 1, 1, 1, 1, 2, 3, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0 }

};


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
 DataBufferPool(int maxnum,int bufsize)
 {
  iMaxNum = maxnum;
  iBufferSize = bufsize;
  iRead = iWrite = 0;
  pData = NULL;
  int res = 0;
  res = sem_init(&hEmpty,0,iMaxNum);  //???????
  if(res != 0)
    {
      iErrorNum = 2;
    }
  res = sem_init(&hUseful,0,0);	 //????????
  if(res != 0)
    {
      iErrorNum = 2;
    }
  pData = new D*[iMaxNum];	 //??????
  for(int i=0;i<iMaxNum;i++)
  {
   pData[i] = new D[iBufferSize];	 //??????
   if (pData[i] == NULL)
   {
    iErrorNum = 1;
   }
  }
  res = pthread_mutex_init(&csDataChange,NULL);
  if(res != 0)
    {
      iErrorNum = 3;
    }
 }
 ~DataBufferPool()
 {
  for (int i=0;i<iMaxNum;i++)
  {
   delete [](pData[i]);
  }
  delete []pData;

  sem_destroy(&hEmpty);
  sem_destroy(&hUseful);
  pthread_mutex_destroy(&csDataChange);

 }
 bool WriteData(void *data,int datasize)
 {

  sem_wait(&hEmpty);  //??????
  pthread_mutex_lock(&csDataChange);//???????,???????
  iWrite %= iMaxNum;
  //////////////////////////////////////////////////////////////////////
  //printf("produce %d \n",iWrite);//???????,???
  //////////////////////////////////////////////////////////////////////
  if(datasize > iBufferSize) return false;
  memcpy((void *)pData[iWrite],(void *)data,datasize*sizeof(D));
  ++iWrite;
  pthread_mutex_unlock(&csDataChange);
  sem_post(&hUseful);
  return true;
 }
 bool ReadData(void *data,int datasize)
 {
  sem_wait(&hUseful);//??????
  pthread_mutex_lock(&csDataChange);
  iRead %= iMaxNum;
  /////////////////////////////////////////////////////////////////////
  //printf("*********consume  %d \n",iRead);//???????,???
  /////////////////////////////////////////////////////////////////////
  if(datasize > iBufferSize) return false;
  memcpy((void *)data,(void *)pData[iRead],datasize*sizeof(D));
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
#define CALLMAX_INTEL 1
#define CHECKMAX_INTEL 1
#define MAXJU      500
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
  int noFlopNum,checkNum,callNum ,raiseNum, allinNum, blindNum, foldNum, noFlodLeastJetton;
}inquireInfo;


class Player
{


public:
  Player(string str);
  void init();
  int setSeat(char *);
  int  getSeatPos(){return m_iSeatPos;}
  vector<string> getSeatInfo() { vector<string> seat ; for(int i=0 ; i<m_iPlayerNum ;i++) seat.push_back(m_sSeatInfo[i]); return seat; }
  void setCard( string card);
  string getCard(int index) { if(index < m_iCardNum) return m_sCard[index]; else return "";}
  string getAction();
  int getPlayNum() {return m_iPlayerNum;}
  int getCardNum() {return m_iCardNum; }
  vector<int> getMaxCardType();
  void initCardType();
  void changeFrontBet(int frontbet);
  GameHis getPlayerHis(string &str);
  void setBlindInfo(vector<string> blindInfo);

  int getMyRemJetton(){return m_iRemainedJetton;}
  void deleaseMyJetton(int lease){ m_iRemainedJetton = m_iMyJetton - lease;}
  void setMyJetton(int jetton){ m_iMyJetton = jetton;}

  int  getMyMoney(){ return m_iMoney[m_iSeatPos];}
  void setHaveBet(int bet) { m_iHaveBet = bet; m_iRemainedJetton = m_iMyJetton - bet;}
  int getHaveBet(){ return m_iHaveBet;}
  int getLeastBet() {int ifrontBet = m_iFrontHaveBet-m_iHaveBet; return (ifrontBet > 0 ? ifrontBet : 0 );}

  bool firstTwoCardIsPair( ){ return m_sCard[1][1] == m_sCard[0][1] ;}
  int getRaiseLeast(){ return m_iRaiseLeastBet;}
//  void clearHist(){ gamehist.clear();}//zhi ji lu yi lun history

  bool getCardStatus(){int i=getCardNum(); if(i ==2 || i >=5) return false; else return true;}

  int addCard(char *info);
  void blind(char *info);
  inquireInfo inquire(char *info);
  void showdown(char *downmsg);

  int getErrorNum(){ return m_iError;}
  int getLunJiShu() {return m_iLunJiShu;}
public:
  bool m_bChangeFrontHaveBet;
  int strConvertInt(char str);
  int gHisPlayNum ;
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
    ADDCARD,BLIND,INQUERE
  };

  void slipInfo( char *info , vector<string> &vData);
  char* removeHead(char *seatInfo, string &rmData);
  void operation(OperateType oType, char *info);
 // vector<GameHis> gamehist;

  int m_iError;//1: card error  2:atoi error
  //tong ji quan ju xin xi
  int m_iLunJiShu ;
  int m_iSumJettonBegin;

};

Player::Player(string str)
{
  init();
  m_sMyID = str;
  m_iLunJiShu = 1;
  m_iSumJettonBegin = 0;
}

void Player::init()
{
  m_sCard.clear();
  for(int i=0 ; i<8 ; i++)
    {

      m_iMoney[i] = 0;
      m_iJetton[i] = 0;
    }
  m_sSeatInfo.clear();
  m_iCardNum = 0;
  m_iPlayerNum = 0;


  m_iRemainedJetton =0;
  m_iRaiseLeastBet = 0;
  m_iLeastBet = 0;
  m_iMyJetton = 0;
  m_iHaveBet = 0;
  m_iFrontHaveBet = 0;
  m_iSeatPos = 0;
  m_vCardType.clear();
  m_iError = 0;

}
void Player::slipInfo( char *info , vector<string> &vData) //end of " " example "bbbb 1111 2222 \n"
{
  char *spacepos = NULL , *infotemp=info;
  string s="";
  while( *infotemp != '\0'  && (NULL != (spacepos = strchr(infotemp, ' '))))
    {
      *spacepos = '\0';
      s = infotemp;
      vData.push_back(s);
      *spacepos = ' ';
      infotemp = spacepos+1;
    }

}

char* Player::removeHead(char *seatInfo, string &rmData)
{
  char *posInfo = NULL;
  if(NULL == (posInfo = strstr(seatInfo, rmData.c_str()))) return NULL;
  return posInfo+strlen(rmData.c_str());
}

void Player::operation(OperateType oType, char *info)
{

  char *findbegin, *findend, *findtemp;
  char operateData[2048]={0};
  memcpy(operateData,info,strlen(info)+1);
  findbegin = operateData;
  vector<string > vData;
  string s="";
  findtemp = strchr(findbegin, '\n');
  findbegin = findtemp+1;
  while (*findbegin != '/')
    {
      s = "";
      vData.clear();
      findend = strchr(findbegin, '\n');
      *findend = '\0';

      slipInfo(findbegin,vData);
      *findend = '\n';
      findbegin = findend+1;
      if(oType == ADDCARD)
        {
          if(vData[0][0] == 'S' || vData[0][0] == 'H' || vData[0][0] == 'C' || vData[0][0] == 'D' )
            {
            setCard(s+vData[0][0]+vData[1][0]);
            }
          else m_iError = 1;
        }
      else if(oType == BLIND)
        {
          setBlindInfo(vData);
        }
      else
        {
          printf("operation error!!");

        }
    }

}

int Player::addCard(char *info)
{
  if(*info == 'h')
    m_iCardNum = 2;
  else if(*info == 'f')
    m_iCardNum = 5;
  else if(*info == 't')
    m_iCardNum = 6;
  else if(*info == 'r')
    m_iCardNum = 7;
  else
    {
      printf("add card error");
      return -1;
    }

  operation(ADDCARD, info);

  if(m_iCardNum == 2 || m_iCardNum >=5)
    {
      initCardType();

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
  printf("blindInfo[1]:%s\n",blindInfo[1].c_str());
  try
  {
    if(blindInfo[0].find(m_sSeatInfo[1]) != string::npos) m_iRaiseLeastBet = atoi(blindInfo[1].c_str())*2;

    if(string::npos ==  blindInfo[0].find(m_sMyID)) return ;

    m_iHaveBet = atoi(blindInfo[1].c_str());
    deleaseMyJetton(m_iHaveBet);
  }
  catch (...)
  {
    m_iError = 2;
  }



}
void Player::changeFrontBet(int frontbet)
{
     m_iFrontHaveBet = frontbet;

}
int Player::setSeat(char *seatInfo)
{
    init();

    string sHead[3]={"button: ", "small blind: ","big blind: "};
    char *findbegin, *findend, *findtemp;
    findbegin = seatInfo;
    vector<string > vData;
    findtemp = strchr(findbegin, '\n');
    findbegin = findtemp+1;
    while (*findbegin != '/')
      {
        vData.clear();
        if(*findbegin == 'b' && *(findbegin+1) == 'u')
          findbegin = findbegin+sHead[0].size();
        else if(*findbegin == 's')
          findbegin = findbegin+sHead[1].size();
        else if(*findbegin == 'b' && *(findbegin+1) == 'i')
          findbegin = findbegin+sHead[2].size();

        findend = strchr(findbegin, '\n');
        *findend = '\0';

        slipInfo(findbegin,vData);
        *findend = '\n';
        findbegin = findend+1;

        m_sSeatInfo.push_back(vData[0]);
        printf("setSeat:vData[1]=%s\n",vData[1].c_str());
         printf("setSeat:vData[2]=%s\n",vData[2].c_str());
         try
         {
           m_iMoney[m_iPlayerNum] = atoi(vData[1].c_str());
           if(m_iLunJiShu == 1) m_iSumJettonBegin += m_iMoney[m_iPlayerNum];
           m_iJetton[m_iPlayerNum] = atoi(vData[2].c_str());
           if(vData[0] == m_sMyID)
             {
               m_iMyJetton = atoi(vData[2].c_str());
               m_iSeatPos = m_iPlayerNum;
             }
            ++m_iPlayerNum;
         }
         catch(...)
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
int Player::strConvertInt(char str)
{
  int i=0;
  switch(str)
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
      i = str-'0';
    }
  return i;
}
void Player::initCardType()
{
  vector<int> typeCard[4] ;//0:SPADES 1:HEARTS 2:CLUBS 3:DIAMONDS
  vector<int> digital ;
  int digiNum[13]={0};
  int shuzi;

  for(int i=0 ; i<m_iCardNum ; i++)
    {
      shuzi = 0;
      shuzi = strConvertInt(m_sCard[i][1]);
      digiNum[shuzi-2]++;
      switch(m_sCard[i][0])
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
          return ;
        }
      digital.push_back(shuzi);
    }
  for(int i=0 ; i<4 ; i++)
    sort(typeCard[i].begin(), typeCard[i].end());
  sort(digital.begin(), digital.end());

// m_vCardType;//dui zi return 1, tong hua return 2, qi ta return 3
  if( 2 == m_iCardNum)
    {
      m_vCardType.clear();
      if (digital[0] == digital[1]) {
          m_vCardType.push_back(1);
          m_vCardType.push_back(digital[0]);
          m_vCardType.push_back(digital[1]);
        }
      else if(typeCard[0].size() ==2 || typeCard[1].size() ==2 || typeCard[2].size() ==2 || typeCard[3].size() ==2)
      {
          m_vCardType.push_back(2);
          m_vCardType.push_back(digital[0]);
          m_vCardType.push_back(digital[1]);
      }
      else
        {
          m_vCardType.push_back(3);
          m_vCardType.push_back(digital[0]);
          m_vCardType.push_back(digital[1]);
        }

    }
  else//5 6 7 first push the sort card and push the card's number of eatch of colour
    {
      m_vCardType.clear();
      for(int i=0 ; i<m_iCardNum; i++)
        {
          m_vCardType.push_back(digital[i]);
        }
      for(int i=0 ; i<4 ; i++)
        {
          m_vCardType.push_back(typeCard[i].size());
        }
      for(int i=0 ; i<13 ; i++)
        {
          m_vCardType.push_back(digiNum[i]);
        }
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
inquireInfo Player::inquire(char *info)
{
  char *findbegin, *findend, *findtemp;
  findbegin = info;
  vector<string > vData;
  string s="";
  findtemp = strchr(findbegin, '\n');
  findbegin = findtemp+1;
  bool bChangeFrontHaveBet = false;

  inquireInfo inqInfo;
  memset(&inqInfo,'\0',sizeof(inqInfo));
  inqInfo.noFlopNum = inqInfo.checkNum = inqInfo.callNum = inqInfo.raiseNum = inqInfo.allinNum = inqInfo.noFlodLeastJetton =0;

  int otherSumBet = 0;
  try
  {
    while (*findbegin != 't')
      {
        s = "";
        vData.clear();
        findend = strchr(findbegin, '\n');
        *findend = '\0';

        slipInfo(findbegin,vData);
        *findend = '\n';
        findbegin = findend+1;
        if(vData[4] != "fold" && bChangeFrontHaveBet == false)
          {
             printf("inquire:vData[3]=%s\n",vData[3].c_str());
            changeFrontBet(atoi(vData[3].c_str()));
            bChangeFrontHaveBet = true;
          }
        if(vData[4] != "fold")
          {
             printf("inquire:vData[1]=%s\n",vData[1].c_str());
            int jetton = atoi(vData[1].c_str());
            inqInfo.noFlodLeastJetton = inqInfo.noFlodLeastJetton > jetton ?jetton:inqInfo.noFlodLeastJetton;
          }
        if(vData[4] == "check")
           ++inqInfo.checkNum;
           else if(vData[4] == "call")
           ++inqInfo.callNum;
           else if(vData[4] == "raise")
           ++inqInfo.raiseNum;
           else if(vData[4] == "all_in")
           ++inqInfo.allinNum;
        else if(vData[4] == "blind")
          ++inqInfo.blindNum;
        else if(vData[4] == "fold")
          ++inqInfo.foldNum;

        otherSumBet += atoi(vData[3].c_str());

      }
    findend = strchr(findbegin, '\n');
    *findend = '\0';
     printf("inqire:totalBet=%s\n",findbegin+strlen("total pot: "));
    int totalBet = atoi(findbegin+strlen("total pot: "));
    *findend = '\n';

    int myHaveBet = totalBet - otherSumBet;
    setHaveBet(myHaveBet);

    inqInfo.noFlopNum = m_iPlayerNum- inqInfo.foldNum;
  }
  catch(...)
  {
    m_iError = 2;
  }
  return inqInfo;

}


bool sockConnect(int &sockfd, struct sockaddr_in *sockaddr , int &len)
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
  while(connect(sockfd, (struct sockaddr *)sockaddr, len) != 0) sleep(1);
  return true;
}







class ActionClass
{
public:
  ActionClass(Player &play);
private:
public:
  int iPlayNum ;
  int mypos ;
  int leastBet ;
  int myRemJetton ;
  int raiseLeastBet ;
  int myMoney ;
  string actionHead[5];
};
ActionClass::ActionClass(Player &play)
{
  iPlayNum = play.getPlayNum();
  mypos = play.getSeatPos();
  leastBet = play.getLeastBet();
  myRemJetton = play.getMyRemJetton();
  myMoney = play.getMyMoney();
  raiseLeastBet = play.getRaiseLeast();
  string actionHead11[5]={" check "," call "," raise "," all_in "," fold "};
  for(int i=0 ; i<5 ;i++)
    {
      actionHead[i] = actionHead11[i];

    }
}

string commonAction(Player &play,inquireInfo &inqInfo,vector<int> &vCardType, ActionClass &action_Int)
{
  if(inqInfo.foldNum == action_Int.iPlayNum -1)
    return action_Int.actionHead[0];
   if(inqInfo.raiseNum ==0 && inqInfo.checkNum == 0 && inqInfo.callNum ==0 && inqInfo.allinNum == 0 && inqInfo.foldNum == 0 && inqInfo.blindNum !=0)
     {
       if(action_Int.myMoney+action_Int.myRemJetton < action_Int.raiseLeastBet)
         return action_Int.actionHead[3];
       if(action_Int.myRemJetton < action_Int.raiseLeastBet)
         return action_Int.actionHead[3];
     }

  return "";
}
string raiseJettonFunc_Crazy(Player &play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  string strCom = commonAction(play,inqInfo,vCardType,action_Int);
  if(strCom != "")
    return strCom;


  char cLeatBet[10]={0};

  memset(cLeatBet,'\0',sizeof(cLeatBet));
  string strLeatBet = "";

  if(action_Int.leastBet == 0)
    {

      if(action_Int.myRemJetton  > action_Int.raiseLeastBet)
        {
          if(lunSum < RAISEMAX_CRAZZY )
            {
              sprintf(cLeatBet,"%d ",action_Int.raiseLeastBet);
              strLeatBet = cLeatBet;
              actionResult = actionResult+action_Int.actionHead[2] + strLeatBet;
              lunSum++;
            }
          else
            {
              if(action_Int.myRemJetton >= inqInfo.noFlodLeastJetton)
                {
                  sprintf(cLeatBet,"%d ",inqInfo.noFlodLeastJetton);
                  strLeatBet = cLeatBet;
                  actionResult = actionResult+action_Int.actionHead[2] + strLeatBet;
                  lunSum++;
                }
              else
                {
                  actionResult = action_Int.actionHead[3];
                }


            }
        }
      else
        {
        actionResult = actionResult +action_Int.actionHead[3];
        }
    }
  else
    {
        if(action_Int.myRemJetton > 10* action_Int.leastBet)
          {
            if(lunSum < RAISEMAX_CRAZZY && inqInfo.raiseNum <= play.getPlayNum()/2)
              {
                sprintf(cLeatBet,"%d ",action_Int.leastBet);
                strLeatBet = cLeatBet;
                actionResult = actionResult+action_Int.actionHead[2] + strLeatBet;
                lunSum++;
              }
            else
              {
                actionResult = actionResult+action_Int.actionHead[1] + strLeatBet;
                lunSum++;
              }

          }
        else if(action_Int.myRemJetton >  action_Int.leastBet)
          {
            actionResult = actionResult +action_Int.actionHead[1];
          }
        else
          {
            actionResult = actionResult +action_Int.actionHead[3];
          }
    }

  return actionResult;
}

string raiseJettonFunc_Intelligent(Player &play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  string strCom = commonAction(play,inqInfo,vCardType,action_Int);
  if(strCom != "")
    return strCom;
  char cLeatBet[10]={0};


  memset(cLeatBet,'\0',sizeof(cLeatBet));
  string strLeatBet = "";

  if(action_Int.leastBet == 0)
    {

      if(action_Int.myRemJetton  > action_Int.raiseLeastBet)
        {
          if(lunSum < RAISEMAX_INTEL)
            {
              sprintf(cLeatBet,"%d ",action_Int.raiseLeastBet);
              strLeatBet = cLeatBet;
              actionResult = actionResult+action_Int.actionHead[2] + strLeatBet;
              lunSum++;
            }
           else
             actionResult = actionResult +action_Int.actionHead[0];
        }
      else
        {
        actionResult = actionResult +action_Int.actionHead[3];
        }
    }
  else
    {
        if(action_Int.myRemJetton > 10* action_Int.leastBet)
          {
            if(lunSum < RAISEMAX_INTEL && inqInfo.raiseNum < action_Int.iPlayNum/2)
              {
                sprintf(cLeatBet,"%d ",action_Int.leastBet);
                strLeatBet = cLeatBet;
                actionResult = actionResult+action_Int.actionHead[2] + strLeatBet;
                lunSum++;
              }
            else
               actionResult = actionResult +action_Int.actionHead[1];

          }
        else if(action_Int.myRemJetton >  action_Int.leastBet)
          {
            actionResult = actionResult +action_Int.actionHead[1];
          }
        else
          {
            actionResult = actionResult +action_Int.actionHead[3];
          }
    }

  return actionResult;
}
string callJettonFunc_Crazy(Player &play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  string strCom = commonAction(play,inqInfo,vCardType,action_Int);
  if(strCom != "")
    return strCom;

  char cLeatBet[10]={0};
  string strLeatBet = "";
  memset(cLeatBet,'\0',sizeof(cLeatBet));
  if(action_Int.leastBet == 0)
    {

      if(action_Int.myRemJetton  > 15*action_Int.raiseLeastBet && inqInfo.noFlopNum <3  )
        {
          sprintf(cLeatBet,"%d ",action_Int.raiseLeastBet);
          strLeatBet = cLeatBet;
          actionResult = actionResult+action_Int.actionHead[2] + strLeatBet;
        }
      else
        {
        actionResult = actionResult +action_Int.actionHead[0];
        }
    }
  else
    {
        if(action_Int.myRemJetton >  action_Int.leastBet)
          {
          actionResult = actionResult +action_Int.actionHead[1];
          }
        else
          {
            actionResult = actionResult +action_Int.actionHead[3];
          }
    }

  return actionResult;
}
string callJettonFunc_Intelligent(Player &play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  string strCom = commonAction(play,inqInfo,vCardType,action_Int);
  if(strCom != "")
    return strCom;
  char cLeatBet[10]={0};
  string strLeatBet ="";


  memset(cLeatBet,'\0',sizeof(cLeatBet));
  if(action_Int.leastBet == 0)
    {
      actionResult = actionResult +action_Int.actionHead[1];

    }
  else
    {
      if(lunSum < CALLMAX_INTEL)
        {
          if(action_Int.leastBet <= (action_Int.myRemJetton+action_Int.myMoney)/10)
            {
              actionResult = actionResult +action_Int.actionHead[1];
            }
          else
            {
              actionResult = actionResult +action_Int.actionHead[4];
            }
          ++lunSum;
        }
      else
        actionResult = actionResult +action_Int.actionHead[4];

    }

  return actionResult;
}

string checkActionFunc_Crazy(Player &play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  string strCom = commonAction(play,inqInfo,vCardType,action_Int);
  if(strCom != "")
    return strCom;

  char cLeatBet[10]={0};

  memset(cLeatBet,'\0',sizeof(cLeatBet));
  if(action_Int.leastBet == 0)
    {
        actionResult = actionResult +action_Int.actionHead[0];

    }
  else
    {
          if(lunSum < CHECKMAX_INTEL)
            {
              if(action_Int.leastBet <= (action_Int.myRemJetton+action_Int.myMoney)/10)
                {
                  actionResult = actionResult +action_Int.actionHead[1];
                }
              else
                {
                  actionResult = actionResult +action_Int.actionHead[4];
                }
              ++lunSum;
            }
          else
            actionResult = actionResult +action_Int.actionHead[4];
     }

  return actionResult;
}

string checkActionFunc_intelligent(Player &play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  string strCom = commonAction(play,inqInfo,vCardType,action_Int);
  if(strCom != "")
    return strCom;

  char cLeatBet[10]={0};

  memset(cLeatBet,'\0',sizeof(cLeatBet));
  if(action_Int.leastBet == 0)
    {
        actionResult = actionResult +action_Int.actionHead[0];

    }
  else
    {

        actionResult = actionResult +action_Int.actionHead[4];
     }


  return actionResult;
}

string fastHandle(Player &play)
{

  string strCard1 = play.getCard(0);
  string strCard2 = play.getCard(1);
  int iCard1 = play.strConvertInt(strCard1[1]);
  int iCard12 = play.strConvertInt(strCard2[1]);

  if(iCard1+iCard12 >= 26)
    return " call ";
  if(strCard1[0] == strCard2[0] && iCard1+iCard12 >=25)
    return " call ";
  if(strCard1[1] == strCard2[1] && iCard1 >= 12)
    return " call ";

  return " fold ";

}

typedef string (*FUNCTION)(Player & , inquireInfo &,vector<int> &) ;
FUNCTION pFun[] = {checkActionFunc_intelligent,checkActionFunc_Crazy,callJettonFunc_Intelligent,
                   callJettonFunc_Crazy,raiseJettonFunc_Intelligent,raiseJettonFunc_Crazy
                    };

string action(Player &play ,inquireInfo & inqInfo)
{

//  if(inqInfo.raiseNum ==0 && inqInfo.checkNum == 0 && inqInfo.callNum ==0 && inqInfo.foldNum == 0 && inqInfo.blindNum !=0)
//    return fastHandle(play);
  ActionClass action_Int(play);
  int iCardNum = play.getCardNum();
  vector<int> vCardType = play.getMaxCardType();


  if(iCardNum == 2)
    {
//      if(vCardType[0] == 1 )//dui zi
//      {
//          if(vCardType[1] >= 12  )
//            {
//              return raiseJettonFunc_Intelligent(play, inqInfo, vCardType);
//            }
//          else if(vCardType[1] >=8)
//            {
//              if(inqInfo.raiseNum <2 )
//                return raiseJettonFunc_Intelligent(play,inqInfo,vCardType);
//              else
//                return callJettonFunc_Intelligent(play,inqInfo,vCardType);

//            }
//          else
//            {
//              if(inqInfo.raiseNum ==0 && inqInfo.noFlopNum < action_Int.iPlayNum/2)
//                return callJettonFunc_Intelligent(play,inqInfo,vCardType);
//              else
//                return checkActionFunc_Crazy(play,inqInfo,vCardType);

//            }


//      }
//      else if(vCardType[0] == 2)//tong hua
//        {
//            if(vCardType[2]+vCardType[1] >= 26  || (vCardType[2]+vCardType[1] >= 25 && vCardType[1] == 12)
//               || (vCardType[2]+vCardType[1] >= 25 && vCardType[1] == 11) )
//             {
//                if(inqInfo.raiseNum ==0 )
//                  return callJettonFunc_Crazy(play,inqInfo,vCardType);
//                else
//                return callJettonFunc_Intelligent(play,inqInfo,vCardType);

//             }
//            else if(vCardType[2]+vCardType[1] >= 24)//A 10 and k J
//             {
//                if(inqInfo.raiseNum == 0)
//                   return  callJettonFunc_Intelligent(play,inqInfo,vCardType);
//                else
//                  return checkActionFunc_Crazy(play,inqInfo,vCardType);

//             }
//            else
//              {

//                if(action_Int.mypos <= 2 && inqInfo.noFlopNum == 3-action_Int.mypos && vCardType[2] >= 11 && vCardType[1]+vCardType[2]>=22)
//                  return callJettonFunc_Intelligent(play,inqInfo,vCardType);
//                if(action_Int.iPlayNum/2 < inqInfo.foldNum || vCardType[2] == 14 )
//                  return checkActionFunc_Crazy(play,inqInfo,vCardType);
//                else
//                  return checkActionFunc_intelligent(play,inqInfo,vCardType);
//              }

//          }
//      else
//        {
//          if(vCardType[2]+vCardType[1] >= 26  || (vCardType[2]+vCardType[1] >= 25 && vCardType[1] == 12)
//             || (vCardType[2]+vCardType[1] >= 25 && vCardType[1] == 11) )
//           {
//              if(inqInfo.raiseNum ==0 )
//                return callJettonFunc_Crazy(play,inqInfo,vCardType);
//              else
//              return callJettonFunc_Intelligent(play,inqInfo,vCardType);

//           }
//           else
//             {
//                  if(inqInfo.raiseNum == 0 && action_Int.mypos <= 2 && inqInfo.noFlopNum == 3-action_Int.mypos && vCardType[2] >= 13 && vCardType[1]+vCardType[2]>=22)
//                    return callJettonFunc_Intelligent(play,inqInfo,vCardType);
//                  if( inqInfo.raiseNum == 0 &&action_Int.iPlayNum/2 < inqInfo.foldNum  && vCardType[2]+vCardType[1] >21 )
//                    return checkActionFunc_Crazy(play,inqInfo,vCardType);
//                  else
//                   return checkActionFunc_intelligent(play,inqInfo,vCardType);
//            }

//        }
      int row = 0;
      for(int i=13; i>vCardType[2]-1;i--)
        row += i;
      row += vCardType[1]-2;
      int iLevel = 0;
      if(inqInfo.noFlopNum >=6)
        {
          if(vCardType[0] == 2)
            {
              iLevel = min(twoCardAction[3][row]-1,5);
            }
          else
            {
              iLevel = min(twoCardAction[0][row]-1,5);

            }
        }
      else if(inqInfo.noFlopNum >=3)
        {
          if(vCardType[0] == 2)
            {
              iLevel = min(twoCardAction[4][row]-1,5);
            }
          else
            {
              iLevel = min(twoCardAction[1][row]-1,5);
            }
        }
      else
        {
          if(vCardType[0] == 2)
            {
              iLevel = min(twoCardAction[5][row]-1,5);
            }
          else
            {
              iLevel = min(twoCardAction[2][row]-1,5);
            }

        }
      printf("Level:%d ",iLevel);
       return pFun[iLevel](play,inqInfo,vCardType);
     }
  if(iCardNum == 5)
    {
      //judge 5 flush
      if(vCardType[iCardNum+0] == 5 || vCardType[iCardNum+1] == 5 ||vCardType[iCardNum+2] == 5 || vCardType[iCardNum+3] == 5)
      return raiseJettonFunc_Crazy(play, inqInfo, vCardType);
      //judge 5 straight
      if(vCardType[iCardNum-1] - vCardType[0] == 4)
        return raiseJettonFunc_Crazy(play, inqInfo, vCardType);
      //judge full house or thread-of-a-kind or two pair
      int firstCheck,secondCheck;
      firstCheck = secondCheck =0;
      for(int i=0; i<13; i++)
        {
          if(vCardType[iCardNum+i+4] >= 2 )
            {
              if(firstCheck == 0) firstCheck = vCardType[iCardNum+i+4];
              else secondCheck = vCardType[iCardNum+i+4];
            }
        }
      // FOURE-OF-A-KIND
      if(firstCheck == 4)
        return raiseJettonFunc_Crazy(play, inqInfo, vCardType);
      //full house
      if((firstCheck+secondCheck) == 5)
        return raiseJettonFunc_Crazy(play, inqInfo, vCardType);
      //htread-of-a-kind
      if((firstCheck+secondCheck) == 3)
        return raiseJettonFunc_Intelligent(play, inqInfo, vCardType);
      //two pair and first two card is not pair
      if((firstCheck+secondCheck == 4) && !play.firstTwoCardIsPair())
        return raiseJettonFunc_Intelligent(play, inqInfo, vCardType);
      //two pair and first two card is pair
      if((firstCheck+secondCheck == 4) && play.firstTwoCardIsPair())
        return callJettonFunc_Crazy(play, inqInfo, vCardType);
      //si tong hua
      if(vCardType[iCardNum+0] == 4 || vCardType[iCardNum+1] == 4 ||vCardType[iCardNum+2] == 4 || vCardType[iCardNum+3] == 4)
      return callJettonFunc_Intelligent(play, inqInfo, vCardType);
      //one pair and first two is not pair
      if((firstCheck+secondCheck == 2) && !play.firstTwoCardIsPair())
        return callJettonFunc_Intelligent(play, inqInfo, vCardType);
      //si sun zi
      if((vCardType[iCardNum-2] - vCardType[0] == 3) || (vCardType[iCardNum-1] - vCardType[1] == 3))
         return checkActionFunc_Crazy(play, inqInfo, vCardType);
      if((firstCheck+secondCheck == 2) && play.firstTwoCardIsPair())
        return checkActionFunc_Crazy(play, inqInfo, vCardType);
      else
        return checkActionFunc_intelligent(play, inqInfo, vCardType);

    }
  else if(iCardNum == 6)
    {

      //judge full house or thread-of-a-kind or two pair
      int firstCheck,secondCheck;
      firstCheck = secondCheck =0;
      for(int i=0; i<13; i++)
        {
          if(vCardType[iCardNum+i+4] >= 2 )
            {
              if(firstCheck == 0) firstCheck = vCardType[iCardNum+i+4];
              else secondCheck = vCardType[iCardNum+i+4];
            }
        }
      if(firstCheck == 4)
        return raiseJettonFunc_Crazy(play, inqInfo, vCardType);
      //full house
      if((firstCheck+secondCheck) == 5)
        return raiseJettonFunc_Crazy(play, inqInfo, vCardType);
      //judge 5 flush
      if(vCardType[iCardNum+0] == 5 || vCardType[iCardNum+1] == 5 ||vCardType[iCardNum+2] == 5 || vCardType[iCardNum+3] == 5)
      return raiseJettonFunc_Intelligent(play, inqInfo, vCardType);
      //judge 5 straight
      if(vCardType[iCardNum-1] - vCardType[0] == 4)
        return raiseJettonFunc_Intelligent(play, inqInfo, vCardType);
      //htread-of-a-kind
      if((firstCheck+secondCheck) == 3)
        return callJettonFunc_Crazy(play, inqInfo, vCardType);
      //two pair and first two card is not pair
      if((firstCheck+secondCheck == 4) && !play.firstTwoCardIsPair())
        return callJettonFunc_Crazy(play, inqInfo, vCardType);
      //two pair and first two card is pair
      if((firstCheck+secondCheck == 4) && play.firstTwoCardIsPair())
        return callJettonFunc_Intelligent(play, inqInfo, vCardType);
      //one pair and first two is not pair
      if((firstCheck+secondCheck == 2) && !play.firstTwoCardIsPair())
        return callJettonFunc_Intelligent(play, inqInfo, vCardType);
      if((firstCheck+secondCheck == 2) && play.firstTwoCardIsPair())
        return checkActionFunc_Crazy(play, inqInfo, vCardType);
      if(vCardType[iCardNum+0] == 4 || vCardType[iCardNum+1] == 4 ||vCardType[iCardNum+2] == 4 || vCardType[iCardNum+3] == 4)
        return checkActionFunc_Crazy(play, inqInfo, vCardType);
      else
        return checkActionFunc_intelligent(play, inqInfo, vCardType);

    }
  else if(iCardNum == 7)
    {
      //judge full house or thread-of-a-kind or two pair
      int firstCheck,secondCheck;
      firstCheck = secondCheck =0;
      for(int i=0; i<13; i++)
        {
          if(vCardType[iCardNum+i+4] >= 2 )
            {
              if(firstCheck == 0) firstCheck = vCardType[iCardNum+i+4];
              else secondCheck = vCardType[iCardNum+i+4];
            }
        }
      if(firstCheck == 4)
        return raiseJettonFunc_Crazy(play, inqInfo, vCardType);
      //full house
      if((firstCheck+secondCheck) == 5)
        return raiseJettonFunc_Crazy(play, inqInfo, vCardType);
      //judge 5 flush
      if(vCardType[iCardNum+0] == 5 || vCardType[iCardNum+1] == 5 ||vCardType[iCardNum+2] == 5 || vCardType[iCardNum+3] == 5)
      return raiseJettonFunc_Intelligent(play, inqInfo, vCardType);
      //judge 5 straight
      if(vCardType[iCardNum-1] - vCardType[0] == 4)
        return raiseJettonFunc_Intelligent(play, inqInfo, vCardType);
      //htread-of-a-kind
      if((firstCheck+secondCheck) == 3)
        return callJettonFunc_Crazy(play, inqInfo, vCardType);
      //two pair and first two card is not pair
      else
        return callJettonFunc_Intelligent(play, inqInfo, vCardType);
    }
  else
    return " fold ";

}
#define MAXREAD 2048
void pot_win(Player &play ,char *argv);
int main(int argc, char *argv[])
{
	if(argc != 6){
	printf("argc error\n");
	return -1;
	}
	int sockfd;
	struct sockaddr_in address , addmy;
	memset(&address , 0 ,sizeof(address));
	memset(&addmy , 0 , sizeof(addmy));
	int result;

	sockfd = socket(AF_INET , SOCK_STREAM ,0);

	addmy.sin_family = AF_INET;
	addmy.sin_addr.s_addr = inet_addr(argv[3]);
	addmy.sin_port = htons((int)(atoi(argv[4])));


	if(-1 == bind(sockfd, (struct sockaddr *)&addmy, sizeof(addmy)))
	{
	  printf("Error : addmy bind failed!\n");
	  return -1;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(argv[1]);
	address.sin_port = htons((int)(atoi(argv[2])));

	int len = sizeof(address);
	if(!sockConnect(sockfd, &address, len)) return -1;

	printf("success connect IP:%s\n", argv[3]);
	char tempbuffer[MAXREAD];
	//  char regbuffer[MAXREAD];
	//char readbuffer[MAXREAD];
	memset(tempbuffer,'\0', MAXREAD);
	//  memset(regbuffer,'\0',MAXREAD);
	//memset(readbuffer,'\0',MAXREAD);

	strcpy(tempbuffer,"reg: ");
	strcat(tempbuffer,argv[5]);
	strcat(tempbuffer," ");
	strcat(tempbuffer,argv[5]);
	strcat(tempbuffer," need_notify ");

	while(0 >= send(sockfd , tempbuffer , strlen(tempbuffer)+1,0)) sleep(1);


	printf("reg success!\n");
	Player play(argv[5]);

	pthread_t a_thread;
	DataBufferPool<char > databuf1(10,MAXREAD);
	DataStruct *threadData = new DataStruct;
	threadData->socknum = sockfd;
	threadData->pSome = (void*)& databuf1;
	int res = pthread_create(&a_thread, NULL, thread_function, (void *)threadData);
	if(res != 0)
	printf("thread create error\n");
	int llll = 0;
	bool bLianPai = false;
	char *tmp  = NULL;
	while(1)
	{
	  bool bOver = false;
	  bool bBenJuOver = false;
	  string s ;
	  play.init();
	  char *pp = NULL;
	  while(1)
	    {

	      if(!bLianPai)
		{
		  memset(tempbuffer,'\0', MAXREAD);
		  databuf1.ReadData((void*)tempbuffer,MAXREAD);
		  tmp= tempbuffer;
		}


		char *findpos = NULL;
		if( (findpos = strstr(tmp,"seat/")) != NULL)
		{
		      *findpos = '\0';

		      if((pp = strstr(tmp,"pot-win/")) != NULL)
		      {
			    printf("%d pot-win\n",llll);
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
                           printf("%d seat\n",llll);
                           pp = strstr(tmp,"/seat");
                           int len = strlen("/seat");
                           char ct = *(pp+len);
                           *(pp+len) = '\0';
                           play.setSeat(findpos);
                           *(pp+len) = ct;
                           bLianPai = false;
                        }





		}
		if((findpos = strstr(tmp,"blind/")) != NULL)
		{
		      printf("%d blind\n",llll);

                      pp = strstr(tmp,"/blind");
                      int len = strlen("/blind");
                      char ct = *(pp+len);
                      *(pp+len) = '\0';
                      play.blind(findpos);
                      *(pp+len) = ct;

		}
		if((findpos = strstr(tmp,"hold/")) != NULL)
		{
		      printf("%d hold\n",llll);

		      pp = strstr(tmp,"/hold");
		      int len = strlen("/hold");
		      char ct = *(pp+len);
		      *(pp+len) = '\0';
		       play.addCard(findpos);
		      *(pp+len) = ct;
		       lunSum = 0;

		}

		if((findpos = strstr(tmp,"flop/")) != NULL)
		{
		      printf("%d flop\n",llll);

		      pp = strstr(tmp,"/flop");
		      int len = strlen("/flop");
		      char ct = *(pp+len);
		      *(pp+len) = '\0';
		      play.addCard(findpos);
		      *(pp+len) = ct;
		      lunSum = 0;

		}
		if((findpos = strstr(tmp,"turn/")) != NULL)
		{
		      printf("%d turn\n",llll);

                      pp = strstr(tmp,"/turn");
                      int len = strlen("/turn");
                      char ct = *(pp+len);
                      *(pp+len) = '\0';
                      play.addCard(findpos);
                      *(pp+len) = ct;
                      lunSum = 0;

		}
		if((findpos = strstr(tmp,"river/")) != NULL)
		{
		      printf("%d river\n",llll);

                      pp = strstr(tmp,"/river");
                      int len = strlen("/river");
                      char ct = *(pp+len);
                      *(pp+len) = '\0';
                      play.addCard(findpos);
                      *(pp+len) = ct;
                      lunSum =0;

		}
		if((findpos = strstr(tmp,"inquire/")) != NULL)
		{
		    printf("%d inquire\n",llll);
		    inquireInfo inq;
		    memset(&inq,'\0',sizeof(inq));

		    pp = strstr(tmp,"/inquire");
		    int len = strlen("/inquire");
		    char ct = *(pp+len);
		    *(pp+len) = '\0';
		    inq =  play.inquire(findpos);
		    *(pp+len) = ct;

		    if(!play.getCardStatus() && play.getErrorNum() == 0)
		    {
		      s = action(play,inq);
		    }
		    else
		    {
		      if(play.gHisPlayNum == inq.foldNum+1)
			s = " check ";
		      else
			s = " fold ";
		    }

                    if(s.size() == 0)
                    printf("action error in inqire!!\n");
                    else
                    printf("action=%s\n",s.c_str());

                    send(sockfd , s.c_str() , s.size()+1,0);

		}
		if((findpos = strstr(tmp,"showdown/")) != NULL)
		{
		      printf("%d showdown\n",llll);
		      pp = strstr(tmp,"/showdown");
		      int len = strlen("/showdown");
		      char ct = *(pp+len);
		      *(pp+len) = '\0';
		     //do sth
		      *(pp+len) = ct;
		}
		if((findpos = strstr(tmp,"pot-win/")) != NULL)
		{
		      printf("%d pot-win\n",llll);

                      pp = strstr(tmp,"/pot-win");
                      int len = strlen("/pot-win");
                      char ct = *(pp+len);
                      *(pp+len) = '\0';
                      //do sth
                      *(pp+len) = ct;

//		      pot_win(play,argv[5]);
		      bBenJuOver = true;
		}
		if((findpos = strstr(tmp,"notify/")) != NULL)
		{
		      printf("%d notify\n",llll);

                      pp = strstr(tmp,"/notify");
                      int len = strlen("/notify");
                      char ct = *(pp+len);
                      *(pp+len) = '\0';
                      //do sth
                      *(pp+len) = ct;
                }
                if((findpos = strstr(tmp,"game-over")) != NULL)
                {
                      printf("%d game-over\n",llll);
                      bBenJuOver = true;
                      bOver = true;
                }
                if((findpos = strstr(tmp,"threadexit")) != NULL)
                {
                      printf("%d game-over\n",llll);
                      bBenJuOver = true;
                      bOver = true;
                }

                if(bBenJuOver) break;

            }

	  if(bOver)
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
   DataBufferPool<char> *bufferPool =(DataBufferPool<char >*)(threadData->pSome);
   int result;
   char tempbuffer[MAXREAD];
   int jishu=0;
   while(1)
     {
       memset(tempbuffer,'\0', MAXREAD);
       result = recv(sockfd, tempbuffer, MAXREAD,0);
       if (result > 0)
         {
           bufferPool->WriteData((void*)tempbuffer,MAXREAD);
         }
       else if(result == 0)
         {
           printf("sock close success!\n");
           bOver = true;
         }
       else
         {
             if(jishu == 3)
             {
             printf("read fail\n");
             bOver = true;
             }
             else{
                 jishu++;
                 continue;
             }


         }


       if(bOver)
         break;

     }
   char pexit[] = "thread exit!\n";
   sprintf(tempbuffer,"%s","threadexit");
   bufferPool->WriteData((void*)tempbuffer,MAXREAD);
   pthread_exit(pexit);

}


void pot_win(Player &play ,char *argv)
{
  FILE *f = NULL;
  static int jishu = 0;
  char path[1024] = {0};
  sprintf(path,"%s%s.txt","/mnt/shared/gamereplay/",argv);
  f = fopen(path ,"a+");
  int iCardNum = play.getCardNum();
  string s="";
  char writeIn[32]={0};
  for(int i=0; i<iCardNum; i++)
    {
      s = play.getCard(i);
      sprintf(writeIn,"%s ",s.c_str());
      fwrite(writeIn, 1, strlen(writeIn)+1, f);
      fflush(f);
    }
  sprintf(writeIn,"%s----%d-----%d\n","----Card",jishu, iCardNum);
  fwrite(writeIn, 1, strlen(writeIn), f);
  fflush(f);
  vector<int> cardtype = play.getMaxCardType();
  for(int i=0;i<cardtype.size();i++)
    {
      sprintf(writeIn,"%d  ",cardtype[i]);
      fwrite(writeIn, 1, strlen(writeIn)+1, f);
      fflush(f);
    }

  sprintf(writeIn,"%s--%d\n","-----CardType",jishu);
  fwrite(writeIn, 1, strlen(writeIn)+1, f);
  fflush(f);
  fclose(f);
  jishu++;

}

