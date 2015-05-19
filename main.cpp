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



//typedef unsigned short int UINTS;
int lunSum = 0;//use in action ,clear in addCard function
#define LUNMAX 5
#define RAISEMAX_CRAZZY 4
#define RAISEMAX_INTEL 3
#define CALLMAX_CRAZY 2
#define CALLMAX_INTEL 1
#define CHECKMAX_INTEL 1
#define MAXREAD 1024

sem_t thread_sem;
sem_t main_sem;
char tempbuffer[MAXREAD];
bool bOver = false;

typedef struct _threadSturct
{
  vector<int> result;

}threadStruct;
void *thread_function(void *arg);

int llll = 0;
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

class Player
{


public:
  Player(string str);
  void init();
  void setSeat(const string &playID, const string &jetton, const string &money);
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

  void setHaveBet(int bet) { m_iHaveBet = bet; m_iRemainedJetton = m_iMyJetton - bet;}
  int getHaveBet(){ return m_iHaveBet;}
  int getLeastBet() {int ifrontBet = m_iFrontHaveBet-m_iHaveBet; return (ifrontBet > 0 ? ifrontBet : 0 );}

  bool firstTwoCardIsPair( ){ return m_sCard[1][1] == m_sCard[0][1] ;}
  int getRaiseLeast(){ return m_iRaiseLeastBet;}
//  void clearHist(){ gamehist.clear();}//zhi ji lu yi lun history
public:
  bool m_bChangeFrontHaveBet;
  int strConvertInt(char str);
private:
  //string m_sCard[7];//0,1 our card , 2 3 4 5 6 public card
  vector<string> m_sCard;
  string m_sSeatInfo[8];//0:button, 1:small blind 2:big blind 3 4 5 6 7 other player
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

 // vector<GameHis> gamehist;
};

Player::Player(string str)
{
  m_sCard.clear();
  for(int i=0 ; i<8 ; i++)
    {
      m_sSeatInfo[i]="";
      m_iMoney[i] = 0;
      m_iJetton[i] = 0;
    }

  m_iCardNum = 0;
  m_iPlayerNum = 0;

  m_sMyID = str;

  m_iLeastBet = 0;
  m_iMyJetton = 0;
  m_iHaveBet = 0;
  m_iFrontHaveBet = 0;
  m_iSeatPos = 0;
}

void Player::init()
{
  m_sCard.clear();
  for(int i=0 ; i<8 ; i++)
    {
      m_sSeatInfo[i]="";
      m_iMoney[i] = 0;
      m_iJetton[i] = 0;
    }

  m_iCardNum = 0;
  m_iPlayerNum = 0;


  m_iRemainedJetton =0;
  m_iRaiseLeastBet = 0;
  m_iLeastBet = 0;
  m_iMyJetton = 0;
  m_iHaveBet = 0;
  m_iFrontHaveBet = 0;
  m_iSeatPos = 0;
}
void Player::setBlindInfo(vector<string> blindInfo)
{
  if(blindInfo[0].find(m_sSeatInfo[1]) != string::npos) m_iRaiseLeastBet = atoi(blindInfo[1].c_str())*2;

  if(string::npos ==  blindInfo[0].find(m_sMyID)) return ;
  m_iHaveBet = atoi(blindInfo[1].c_str());

  deleaseMyJetton(m_iHaveBet);
}
void Player::changeFrontBet(int frontbet)
{


      m_iFrontHaveBet = frontbet;

}
void Player::setSeat(const string &playID, const string &jetton, const string &money)
{
  m_sSeatInfo[m_iPlayerNum] = playID;
  m_iMoney[m_iPlayerNum] = atoi(money.c_str());
  m_iJetton[m_iPlayerNum] = atoi(jetton.c_str());

  if(playID == m_sMyID)
    {
      m_iMyJetton = atoi(jetton.c_str());
      m_iSeatPos = m_iPlayerNum;
    }

  ++m_iPlayerNum;

}
void Player::setCard(string card)
{
//  m_sCard[m_iCardNum] = card;
  m_sCard.push_back(card);
  ++m_iCardNum;
  if(m_iCardNum == 2 || m_iCardNum >=5)
    {
      initCardType();

    }
  printf("%d hand ::%s \n",llll,card.c_str());
  fflush(stdout);
//  printf("\n");
//  char pp[256] ={0};
//  sprintf(pp,"/home/game/huawei/sshcpy/clientCardReplay/%s.txt",m_sMyID.c_str());
//   FILE *f = fopen(pp ,"w+");

//   fseek(f, 0 ,SEEK_END);
//   fwrite(card.c_str(), 1,card.size()+1, f);
//   fflush(f);
//   fclose(f);

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
  int typeCard[4]={0} ;//0:SPADES 1:HEARTS 2:CLUBS 3:DIAMONDS
  vector<int> digital ;
  int digiNum[13]={0};
  int shuzi;

  for(int i=0 ; i<m_iCardNum ; i++)
    {
      shuzi = strConvertInt(m_sCard[i][1]);
      digiNum[shuzi-2]++;
      switch(m_sCard[i][0])
        {
        case 'S':
          typeCard[0]++;
          break;
        case 'H':
          typeCard[1]++;
          break;
        case 'C':
          typeCard[2]++;
          break;
        case 'D':
          typeCard[3]++;
          break;
        default:
          perror("card error!\n");
        }
      digital.push_back(shuzi);
    }
  sort(digital.begin(), digital.end());

  vector<int> returnresult;//dui zi return 1, tong hua return 2, qi ta return 3
  m_vCardType.clear();
  if( 2 == m_iCardNum)
    {
      returnresult.clear();
      if (digital[0] == digital[1]) {
          returnresult.push_back(1);
          returnresult.push_back(digital[0]);
          returnresult.push_back(digital[1]);
        }
      else if(typeCard[0]==2 || typeCard[1] ==2 || typeCard[2] ==2 || typeCard[3] ==2)
      {
          returnresult.push_back(2);
          returnresult.push_back(digital[0]);
          returnresult.push_back(digital[1]);
      }
      else
        {
          returnresult.push_back(3);
          returnresult.push_back(digital[0]);
          returnresult.push_back(digital[1]);
        }

    }
  else//5 6 7 first push the sort card and push the card's number of eatch of colour
    {
      returnresult.clear();
      for(int i=0 ; i<m_iCardNum; i++)
        {
          returnresult.push_back(digital[i]);
        }
      for(int i=0 ; i<4 ; i++)
        {
          returnresult.push_back(typeCard[i]);
        }
      for(int i=0 ; i<13 ; i++)
        {
          returnresult.push_back(digiNum[i]);
        }
    }

  m_vCardType = returnresult;
  printf(" %d hand:::::::" ,llll);
  for(int i=0; i<returnresult.size() ;i++)
    printf("%d",returnresult[i]);
  printf("\n");
  fflush(stdout);


}
vector<int> Player::getMaxCardType()
{


  return m_vCardType;

}
void pot_win(Player &play,char *argv);
enum OperateType
{
  ADDCARD,BLIND,INQUERE
};
void slipInfo( char *info , vector<string> &vData) //end of " " example "bbbb 1111 2222 \n"
{
  char *spacepos = NULL , *infotemp=info;
  string s="";
  while(NULL != (spacepos = strchr(infotemp, ' ')))
    {
      *spacepos = '\0';
      s = infotemp;
      vData.push_back(s);
      *spacepos = ' ';
      infotemp = spacepos+1;
    }

}

char* removeHead(char *seatInfo, string &rmData)
{
  char *posInfo = NULL;
  if(NULL == (posInfo = strstr(seatInfo, rmData.c_str()))) return NULL;
  return posInfo+strlen(rmData.c_str());
}



int seat(char *seatInfo, Player &play)
{
  llll ++;
  play.init();

  string sHead[3]={"button: ", "small blind: ","big blind: "};
  char *findbegin, *findend, *findtemp;
  findbegin = seatInfo;
  vector<string > vData;
  string s="";
  findtemp = strchr(findbegin, '\n');
  findbegin = findtemp+1;
  while (*findbegin != '/')
    {
      s = "";
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
       play.setSeat(vData[0], vData[1], vData[2]);

    }

  return play.getPlayNum();
}

void showdown(char *downmsg, Player &play)
{

}

void operation(OperateType oType, char *info, Player &play)
{
  char *findbegin, *findend, *findtemp;
  findbegin = info;
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
              play.setCard(s+vData[0][0]+vData[1][0]);
            }
          else perror("add card error!");
        }
      else if(oType == BLIND)
        {
          play.setBlindInfo(vData);
        }

    }

}
int addCard(char *info ,Player &play)
{
  operation(ADDCARD, info, play);
  if(play.getCardNum() > 2) lunSum =0;
  return 0;
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


typedef struct _inquireInfo{
  int noFlopNum,checkNum,callNum ,raiseNum, allinNum, blindNum, foldNum, noFlodLeastJetton;
}inquireInfo;




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
  string actionHead[5];
};
ActionClass::ActionClass(Player &play)
{
  iPlayNum = play.getPlayNum();
  mypos = play.getSeatPos();
  leastBet = play.getLeastBet();
  myRemJetton = play.getMyRemJetton();
  raiseLeastBet = play.getRaiseLeast();
  string actionHead11[5]={" check "," call "," raise "," all_in "," fold "};
  for(int i=0 ; i<5 ;i++)
    {
      actionHead[i] = actionHead11[i];

    }
}

string raiseJettonFunc_Crazy(Player &play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  if(inqInfo.foldNum == action_Int.iPlayNum -1)
    return action_Int.actionHead[0];
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
              sprintf(cLeatBet,"%d ",inqInfo.noFlodLeastJetton);
              strLeatBet = cLeatBet;
              actionResult = actionResult+action_Int.actionHead[2] + strLeatBet;
              lunSum++;
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
                sprintf(cLeatBet,"%d ",action_Int.leastBet);
                strLeatBet = cLeatBet;
                actionResult = actionResult+action_Int.actionHead[2] + strLeatBet;
                lunSum++;
              }

          }
        else if(action_Int.myRemJetton >  action_Int.leastBet)
          {
            actionResult = actionResult +action_Int.actionHead[0];
          }
        else
          {
            actionResult = actionResult +action_Int.actionHead[3];
          }
    }

  return actionResult;
}

string raiseJettonFunc_Intelligent(Player play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  char cLeatBet[10]={0};
  if(inqInfo.foldNum == action_Int.iPlayNum -1)
    return action_Int.actionHead[0];

  memset(cLeatBet,'\0',sizeof(cLeatBet));
  string strLeatBet = "";

  if(action_Int.leastBet == 0)
    {

      if(action_Int.myRemJetton  > action_Int.raiseLeastBet)
        {
          if(lunSum < RAISEMAX_INTEL && inqInfo.raiseNum < 3)
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
            if(lunSum < RAISEMAX_INTEL && inqInfo.raiseNum < 3)
              {
                sprintf(cLeatBet,"%d ",action_Int.leastBet);
                strLeatBet = cLeatBet;
                actionResult = actionResult+action_Int.actionHead[2] + strLeatBet;
                lunSum++;
              }
            else
               actionResult = actionResult +action_Int.actionHead[0];

          }
        else if(action_Int.myRemJetton >  action_Int.leastBet)
          {
            actionResult = actionResult +action_Int.actionHead[0];
          }
        else
          {
            actionResult = actionResult +action_Int.actionHead[3];
          }
    }

  return actionResult;
}
string callJettonFunc_Crazy(Player play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  if(inqInfo.foldNum == action_Int.iPlayNum -1)
    return action_Int.actionHead[0];

  char cLeatBet[10]={0};
  string strLeatBet = "";
  memset(cLeatBet,'\0',sizeof(cLeatBet));
  if(action_Int.leastBet == 0)
    {

      if(action_Int.myRemJetton  > 15*action_Int.raiseLeastBet && inqInfo.raiseNum == 0 && inqInfo.noFlopNum <3  )
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
string callJettonFunc_Intelligent(Player play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  char cLeatBet[10]={0};
  string strLeatBet ="";

  if(inqInfo.foldNum == action_Int.iPlayNum -1)
    return action_Int.actionHead[0];

  memset(cLeatBet,'\0',sizeof(cLeatBet));
  if(action_Int.leastBet == 0)
    {
      actionResult = actionResult +action_Int.actionHead[1];

    }
  else
    {
      if(lunSum < CALLMAX_INTEL)
        {
          if(action_Int.leastBet <= action_Int.myRemJetton/4)
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

string checkActionFunc_Crazy(Player play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  char cLeatBet[10]={0};

  if(inqInfo.foldNum == action_Int.iPlayNum -1)
    return action_Int.actionHead[0];

  memset(cLeatBet,'\0',sizeof(cLeatBet));
  if(action_Int.leastBet == 0)
    {
        actionResult = actionResult +action_Int.actionHead[0];

    }
  else
    {
          if(lunSum < CHECKMAX_INTEL)
            {
              if(action_Int.leastBet <= action_Int.myRemJetton/10)
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

string checkActionFunc_intelligent(Player play, inquireInfo &inqInfo,vector<int> &vCardType)
{
  string actionResult="";

  ActionClass action_Int(play);

  char cLeatBet[10]={0};

  if(inqInfo.foldNum == action_Int.iPlayNum -1)
    return action_Int.actionHead[0];
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


string action(Player &play ,inquireInfo & inqInfo)
{
  if(inqInfo.raiseNum ==0 && inqInfo.checkNum == 0 && inqInfo.callNum ==0 && inqInfo.foldNum == 0 && inqInfo.blindNum !=0)
    return fastHandle(play);
  ActionClass action_Int(play);
  int iCardNum = play.getCardNum();
  vector<int> vCardType = play.getMaxCardType();


  if(iCardNum == 2)
    {
      if(vCardType[0] == 1 )//dui zi
      {
          if(vCardType[1] >= 12  )
            {
              return raiseJettonFunc_Crazy(play, inqInfo, vCardType);
            }
          else if(vCardType[1] >=8)
            {
              if(inqInfo.raiseNum <2 )
                return raiseJettonFunc_Crazy(play,inqInfo,vCardType);
              else
                return callJettonFunc_Intelligent(play,inqInfo,vCardType);

            }
          else
            {
              if(inqInfo.raiseNum ==0 && inqInfo.noFlopNum < action_Int.iPlayNum/2)
                return callJettonFunc_Crazy(play,inqInfo,vCardType);
              else
                return checkActionFunc_Crazy(play,inqInfo,vCardType);

            }


      }
      else if(vCardType[0] == 2)//tong hua
        {
            if(vCardType[2]+vCardType[1] >= 26  )
             {
                if(inqInfo.raiseNum ==0 )
                  return raiseJettonFunc_Crazy(play,inqInfo,vCardType);
                else
                return raiseJettonFunc_Intelligent(play,inqInfo,vCardType);

             }
            else if(vCardType[2]+vCardType[1] >= 25 && vCardType[1] == 12)//K Q
             {
                if(inqInfo.raiseNum == 0)
                  return raiseJettonFunc_Intelligent(play,inqInfo,vCardType);
                else
                 return callJettonFunc_Crazy(play,inqInfo,vCardType);
             }
            else if((vCardType[2]+vCardType[1] >= 25 && vCardType[1] == 11) )//A J
             {
                if(inqInfo.raiseNum == 0)
                return  callJettonFunc_Crazy(play,inqInfo,vCardType);
                else
                 return callJettonFunc_Intelligent(play,inqInfo,vCardType);
             }
            else if(vCardType[2]+vCardType[1] >= 24)//A 10 and k J
             {
                if(inqInfo.raiseNum == 0)
                   return  callJettonFunc_Intelligent(play,inqInfo,vCardType);
                else
                  return checkActionFunc_Crazy(play,inqInfo,vCardType);

             }
            else
              {

                if(action_Int.mypos <= 2 && inqInfo.noFlopNum == 3-action_Int.mypos && vCardType[2] >= 11 && vCardType[1]+vCardType[2]>=22)
                  return callJettonFunc_Intelligent(play,inqInfo,vCardType);
                if(action_Int.iPlayNum/2 < inqInfo.foldNum || vCardType[2] == 14 )
                  return checkActionFunc_Crazy(play,inqInfo,vCardType);
                else
                  return checkActionFunc_intelligent(play,inqInfo,vCardType);
              }

          }
      else
        {
               if(vCardType[2]+vCardType[1] >= 26  )
                {
                   if(inqInfo.raiseNum ==0 )
                     return raiseJettonFunc_Intelligent(play,inqInfo,vCardType);
                   else
                   return callJettonFunc_Crazy(play,inqInfo,vCardType);

                }
               else if(vCardType[2]+vCardType[1] >= 25 && vCardType[1] == 12)//K Q
                {
                   if(inqInfo.raiseNum == 0)
                     return callJettonFunc_Crazy(play,inqInfo,vCardType);
                   else
                    return callJettonFunc_Intelligent(play,inqInfo,vCardType);
                }
               else if((vCardType[2]+vCardType[1] >= 25 && vCardType[1] == 11) )//A J
                {
                   if(inqInfo.raiseNum == 0 && inqInfo.noFlopNum == 3-action_Int.mypos )
                    return  callJettonFunc_Intelligent(play,inqInfo,vCardType);
                   else
                    return checkActionFunc_Crazy(play,inqInfo,vCardType);
                }
               else
                 {
                      if(inqInfo.raiseNum == 0 && action_Int.mypos <= 2 && inqInfo.noFlopNum == 3-action_Int.mypos && vCardType[2] >= 13 && vCardType[1]+vCardType[2]>=22)
                        return callJettonFunc_Intelligent(play,inqInfo,vCardType);
                      if( inqInfo.raiseNum == 0 &&action_Int.iPlayNum/2 < inqInfo.foldNum  && vCardType[2]+vCardType[1] >21 )
                        return checkActionFunc_Crazy(play,inqInfo,vCardType);
                      else
                       return checkActionFunc_intelligent(play,inqInfo,vCardType);
                }

        }
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
        return raiseJettonFunc_Crazy(play, inqInfo, vCardType);
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
      else
        return checkActionFunc_intelligent(play, inqInfo, vCardType);

    }
  else
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
        return callJettonFunc_Intelligent(play, inqInfo, vCardType);
      //two pair and first two card is pair
      if((firstCheck+secondCheck == 4) && play.firstTwoCardIsPair())
        return checkActionFunc_Crazy(play, inqInfo, vCardType);
      //one pair and first two is not pair
      if((firstCheck+secondCheck == 2) && !play.firstTwoCardIsPair())
        return checkActionFunc_intelligent(play, inqInfo, vCardType);
      if((firstCheck+secondCheck == 2) && play.firstTwoCardIsPair())
        return checkActionFunc_intelligent(play, inqInfo, vCardType);
      else
        return checkActionFunc_intelligent(play, inqInfo, vCardType);
    }

}


string inquire(char *info , Player &play)
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
          play.changeFrontBet(atoi(vData[3].c_str()));
          bChangeFrontHaveBet = true;
        }
      if(vData[4] != "fold")
        {
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
  int totalBet = atoi(findbegin+strlen("total pot: "));
  *findend = '\n';

  int myHaveBet = totalBet - otherSumBet;
  play.setHaveBet(myHaveBet);

  inqInfo.noFlopNum = play.getPlayNum() - inqInfo.foldNum;
  return action(play,inqInfo);

}


void blind(Player &play , char *info)
{
  operation(BLIND, info, play);
}


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

  char regbuffer[MAXREAD];
  char readbuffer[MAXREAD];
  memset(tempbuffer,'\0', MAXREAD);
  memset(regbuffer,'\0',MAXREAD);
  memset(readbuffer,'\0',MAXREAD);

  strcpy(tempbuffer,"reg: ");
  strcat(tempbuffer,argv[5]);
  strcat(tempbuffer," ");
  strcat(tempbuffer,argv[5]);
  strcat(tempbuffer," ");

//  memcpy(regbuffer,tempbuffer,strlen(tempbuffer)+1);

  if(-1 == write(sockfd , tempbuffer , strlen(tempbuffer)+1))
  {
   printf("write failed\n");
   return -1;
  }


  int res;
  res = sem_init(&thread_sem,0,0);
  if(res != 0)
    printf("semp error\n");
  res = sem_init(&main_sem,0,0);
  if(res !=0)
    printf("semp error\n");
  pthread_t a_thread;
  res = pthread_create(&a_thread, NULL, thread_function, (void *)&sockfd);
  if(res != 0)
    printf("thread create error\n");
  while(1)
    {
      string s ;
      Player play(argv[5]);

//      f = fopen("./clientreplay.txt" ,"w+");


      while(1)
        {
            sem_wait(&thread_sem);
            memcpy(readbuffer,tempbuffer,MAXREAD);
            sem_post(&main_sem);
            int ipos = 0;
            while(!isalpha(*readbuffer)) ipos++;
            switch(readbuffer[ipos])
              {
              case 's'://two select ,,,warning 1:seat 2:showdown
                if(readbuffer[ipos+1] == 'e')
                  {
                    if(seat(readbuffer+ipos,play) == 0)
                      {
                        bOver = true;
                        break;
                      }
                  }
                 else
                  showdown(readbuffer+ipos,play);
                break;
              case 'b':
                blind(play, readbuffer+ipos);
                break;
              case 'h':
              case 'f':
              case 't':
              case 'r':
                addCard(readbuffer+ipos, play);
                break;
              case 'i':
                {


                  printf("%d success inquire ! strlen = %d\n",llll,strlen(readbuffer));
                  fflush(stdout);
//                  if(play.getCard(0) == "")
//                    s = " fold ";
//                  else
//                    s = inquire(readbuffer, play);

                  s = inquire(readbuffer+ipos, play);
                  if(s.size() == 0)
                    printf("action error in inqire!!\n");
                  else
                    printf("action=%s\n",s.c_str());
                  for(int i=0 ; i<3 ; i++)
                    {
                     if(0 < write(sockfd , s.c_str() , s.size()+1))
                       break;
                     else if(i==2) return -1;
                      sleep(1);
                    }


                  break;
                }

              case 'p':
                pot_win(play,argv[5]);
                break;
              case 'g':
                bOver = true;
                break;
               default:
                break;

              }

            if(bOver) break;

      }
      if(bOver)
        {
          printf("game over !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
          break;
        }
//      fclose(f);

    }
  void *threadresult;
  pthread_join(a_thread,&threadresult);
  close(sockfd);
  return 0;
}

void *thread_function(void *arg)
{
  bool bOver = false;
  int result;
  int sockfd = *((int*)arg);
  while(1)
    {
      memset(tempbuffer,'\0', MAXREAD);

      for(int i=0 ; i<3 ;i++)
        {
         result = read(sockfd, tempbuffer, MAXREAD);

         if (result > 0) break;
         if(i != 2) continue;
         else
            {
              printf("read fail\n");
              bOver = true;
              break;
            }
        }
      sem_post(&thread_sem);
      if(bOver == true) break;

      sem_wait(&main_sem);

    }
  char pexit[] = "thread exit!\n";
  pthread_exit(pexit);

}

void pot_win(Player &play ,char *argv)
{
  FILE *f = NULL;
  static int jishu = 0;
  char path[1024] = {0};
  sprintf(path,"%s%s.txt","/home/game/huawei/sshcpy/",argv);
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
  sprintf(writeIn,"%s----%d\n","----Card",jishu);
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
