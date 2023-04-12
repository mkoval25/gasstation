#include <iostream>
#include <random>
#include <list>
#include <queue>

#include "constants.h"

using namespace std;

// Model -> Request <- Column -<> GasStation -<> Model

// все время меряется в минутах


enum GasType { A11, B11, A12, B12 };

enum Status {Pending, Accepted, Declined};


class Request {
    public:
        GasType gType;
        int volume; // объем в литрах
        int colNum; // номер колонны

        int timeOfService; // время обслуживания
        int status; // не принята - 0 / принята - 1 / отклонена - 2

        Request(GasType gType, int volume, int colNum, int timeOfService) {
            this->gType = gType;
            this->volume = volume;
            this->colNum = colNum;
            this->timeOfService = timeOfService;
            this->status = Pending; // сразу после создания заявка еще не принята
        }
};


class Column {
    public:

        int N; // макс. длина очереди
        // int numOfCars; // текущее кол-во машин в колонне

        // двунаправленная очередь
        deque<Request> currReqs;
        // int t_accepted; // время принятия последней заявки
        // int t_service; // время обслуживания последней заявки, множитель*volume

        Column(int N) {
            this->N = N;
        }

        int queueNum() {return currReqs.size();}

        //void tick() {} // if t_accepted + t_service >= currTime, то +1 обслуженная заявка и -1 машина из очереди


        void addReqToQueue(Request req) {
            // ReqQueue.push(req)
            currReqs.push_back(req);
        }

};


class GasStation {
    public:

        int N;
//      int K;
        vector<Column> cols; // массив колонн

        // int stocks[B12]; // unused

        GasStation() {
            this->N = 10;
        }

        GasStation(int N, int K, int margin) {
            this->N = N;

            for (int i = 0; i < K; i++) {

                cols.push_back(Column(N));
            }
        }

        bool ServiceReq(Request req) {
            if (cols[req.colNum].queueNum() < this->N) {
                cols[req.colNum].addReqToQueue(req);
                return true;
            }
            cout << "no room for " << req.colNum << "!" << endl;
            return false; // no free places in queue...
        }

        void UpdateReqsTime(int t_mins) { // updating the remaining timeOfService for the first req in queue

            for (Column& col : cols) { // TODO
                cout << "COL ";

                cout << col.queueNum();
                if (!col.currReqs.empty()) {
                    Request req1 = col.currReqs.front();
                    cout << " time is: " << req1.timeOfService;
                    req1.timeOfService = max(req1.timeOfService-=t_mins, 0);
                    col.currReqs.pop_front();
                    col.currReqs.push_front(req1);
                    cout << " time left: " << req1.timeOfService;
                }
                cout << endl;
            }
        };

        void ClearServicedReqs() {
            // remove all requests with t_service = 0...
            for (Column& col : cols) { // TODO
                //cout << "currReqs len: " << col.currReqs.size() << endl;
                if (!col.currReqs.empty()) {
                    Request req1 = col.currReqs.front();
                    if (req1.timeOfService == 0) {
                        col.currReqs.pop_front();
                        cout << "req done! popped from front" << endl;
                    }
                }
            }
        }

};

// Генераторы случайных чисел

int UniRandom(int min, int max) { // равномерное распределение
    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> uni(min,max);

    auto random_int = uni(rng);

    return random_int;
}

int NormalRandom(int mean, int stddev) { // равномерное распределение
    random_device rd;
    mt19937 gen(rd());

    normal_distribution<> norm(mean, stddev);

    auto random_int = norm(gen);
    return random_int;
}

class Model {
// interacts with GUI
public:

    int currTime[2] = {0,0}; // (день недели = 0..6, время суток = 0..1440 минут), заканчива
    int modellingStep; //В начале, время 0, потом каждый шаг добавляет modellingStep минут
    int N; // макс. длина очереди
    int K; // макс. кол-во колонн

    int margin; // replace to GasStation class
    int prices[4];

    int meanTime; // время между заявками, должно зависеть от наценки: больше наценка -> больше meanTime
    int stddev;

    float weekendCoeff;
    float hourCoeff;

    float dayHourCoeffs[7][24];
    float marginCoeff;

    GasStation gasStation;

    list<Request> ReqList;

    //int timeForLiter = 20; // 20 secs;

    // stats
    int overallProfit = 0;
    int avgVolume[4] = {0,0,0,0};
    int numAccepted = 0;
    int numDeclined = 0;

    // временной интервал с последней заявки
    int timeFromLastReq = 0;

    Model(int modellingStep, int N, int K, int margin) {
        this->modellingStep = modellingStep;
        this->N = N;
        this->K = K;

        this->margin = margin;

        HoursCoeffsInitialize(); // иницализировали массив dayHourCoeffs

        this->meanTime = defaultMeanTime;
        this->stddev = 3; // константа
        //timeForLiter = 20; // 20 sec

        this->prices[0] = defaultPrices[0]*(100+margin)/100;
        this->prices[1] = defaultPrices[1]*(100+margin)/100;
        this->prices[2] = defaultPrices[2]*(100+margin)/100;
        this->prices[3] = defaultPrices[3]*(100+margin)/100;

        ReqList = list<Request>();
        this->gasStation = GasStation(N, K, margin);

    }

    // генератор заявок включен в класс Model
    Request generateNewReq() {

        GasType gType = GasType(UniRandom(0,4));
        int colNum = UniRandom(0,K-1);
        int volume = UniRandom(10,50); // в литрах
        int timeOfService = defaultTimeOfService + volume * timeForLiter/60;

        return Request(gType,volume,colNum,timeOfService);
    }

    int intervalCram(int time) { // time in mins, from 1 to 20 mins
        int time_;
        time_ = max(time,1);
        time_ = min(20, time_);

        return time_;
    }

    bool IsWeekEnd() {
        if (currTime[0] > 4) {
            return true;
        }
        return false;
    }

    int ModifyMeanTime() { // реализация логики изменения мат. ожидания интервала между заявками
        weekendCoeff = (IsWeekEnd()) ? 0.9 : 1.0; // по выходным чаще поступают заявки
        int day_ndx = currTime[0];
        int hour_ndx = currTime[1]/60;
        if (currTime[1] >= 1440) {
            hour_ndx -= 1;
        };

        hourCoeff = dayHourCoeffs[day_ndx][hour_ndx];
        marginCoeff = margin/defaultMargin;

        //int _mean = defaultMeanTime * weekendCoeff * hourCoeff * marginCoeff;
        int _mean = defaultMeanTime * weekendCoeff * hourCoeff * (100+marginCoeff)/100;

        return _mean;
    }


    void Tick() { // called every step
        int stepTime = 0;
        int d_time = 0;
        int currMeanTime = meanTime;

        // generating and accepting Requests for one modelling step...
        while(true) {

            // d_time generating from 1 to 20 mins...
            currMeanTime = ModifyMeanTime();

            if (timeFromLastReq != 0) {
                d_time = timeFromLastReq;
                timeFromLastReq = 0;
            } else {
                d_time = NormalRandom(currMeanTime,stddev);
                d_time = intervalCram(d_time);
            }

            cout << "currMeanTime: " << currMeanTime << endl;
            cout << "gen d_time: " << d_time << endl;

            stepTime += d_time;

            if (stepTime >= modellingStep) {
                timeFromLastReq = stepTime - modellingStep;
                break;

            }

            TimeTick(d_time);

            Request req = generateNewReq();

            if (gasStation.ServiceReq(req)) {
                req.status = Accepted;
            }
            else {
                req.status = Declined;
            }

            ReqList.push_back(req); // adding new request to a list

            gasStation.UpdateReqsTime(d_time); // only the head of queue
            gasStation.ClearServicedReqs(); // pop the head of queue if service time's over
        }

        int leftT = modellingStep - stepTime + d_time;

        gasStation.UpdateReqsTime(leftT);

        gasStation.ClearServicedReqs();

        TimeTick(leftT);
        cout << "weekday:" << currTime[0] << endl;
        cout << "TIME: " << currTime[1] << endl;
    }

    void TimeTick(int t_mins) { // updating the modelling time
        currTime[1] += t_mins;
        if (currTime[1] > minsInDay) {
            currTime[0] += 1;
            currTime[1] -= minsInDay; // обнулили время в сутках, тк начались новые
        }
    };

    void HoursCoeffsInitialize () {
        for (int d = 0; d < daysInWeek; d++) {
            for (int h = 0; h < 24; h++ ) {
                if (h >= 8) { // 8, так как начинается с нуля
                    if (h < 13) { // 9..14 
                        dayHourCoeffs[d][h] = 0.6;
                    } else {
                        if (h < 18 ) { // 14 .. 19 - меньше
                            dayHourCoeffs[d][h] = 0.9;
                        } else {
                            if (h <= 21) {// 19..22 - по будням больше, по выходным меньше
                                if (d > 4) {
                                    dayHourCoeffs[d][h] = 1.0;
                                } else {
                                    dayHourCoeffs[d][h] = 0.5;
                                }
                            } else { // 22..24 - затишье, как и с 0..9
                                dayHourCoeffs[d][h] = 1.2; 
                            }
                        }
                    }
                } else { // 0..9
                    dayHourCoeffs[d][h] = 1.2; 
                }
            }
        }
        
    }

    void GetStats() { // проходимся по списку всех заявок и собираем статистику, вызывая соотв. функции
        overallProfit = GetOverallProfit();
        GetAvgVolumeByTypes();
        numAccepted = GetNumOfAccepted();
        numDeclined = GetNumOfDeclined();

    }

    int GetOverallProfit() {
        int profit = 0;

         for (Request& req : ReqList) {
            if (req.status == Accepted) {
                profit += req.volume * prices[req.gType];
            }
        }
        return profit;

    }

    void GetAvgVolumeByTypes() {
        for (int i = 0; i < 4; i++) {
            int sumVolume = 0;
            int numVolume = 0;
            for (Request& req : ReqList) {
                if (req.gType == i) {
                    sumVolume += req.volume;
                    numVolume += 1;
                }
            }
            if(numVolume != 0)
                avgVolume[i] = sumVolume / numVolume;
        }
    }

    int GetNumOfAccepted() {
        int num = 0;

        for (Request& req : ReqList) {
           if (req.status == Accepted) {
                num += 1;
           }
        };
        return num;
    }

    int GetNumOfDeclined() {
        int num = 0;
        for (Request& req : ReqList) {
           if (req.status == Declined) {
                num += 1;
           }
        };
        return num;
    }
};
