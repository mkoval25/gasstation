#include <iostream>
#include <random>
#include <list>
#include <queue>

#include "constants.h"

using namespace std;

// Model -> Request <- Column -<> GasStation -<> Model

// все время меряется в минутах

// TODO:

// разбить классы на файлы
// Моделирование должно идти одну неделю

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

        int margin;
        int prices[B12];
        // int stocks[B12]; // unused

        GasStation() {
            this->N = 10;
            this->margin = 10;
        }

        GasStation(int N, int K, int margin) {
            this->N = N;
            this->margin = margin;


            for (int i = 0; i < K; i++) {

                cols.push_back(Column(N));
            }

            this->prices[A11] = 10;
            this->prices[A12] = 15;
            this->prices[B11] = 20;
            this->prices[B12] = 22;
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

    normal_distribution<> norm{5, 2};

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

    int meanTime; // время между заявками, должно зависеть от наценки: больше наценка -> больше meanTime
    int stddev;


    GasStation gasStation;

    list<Request> ReqList;

    int timeForLiter = 20; // 20 secs;

    Model(int modellingStep, int N, int K, int margin) {
        this->modellingStep = modellingStep;
        this->N = N;
        this->K = K;

        this->margin = margin;

        this->meanTime = 10;
        this->stddev = 10; // константа
        timeForLiter = 20; // 20 sec

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

    void Tick() { // called every step
        int stepTime = 0;
        if(isWeekEnd()) {
            meanTime = 5;
        } else {
            meanTime = 10;
        }
        // generating and accepting Requests for one modelling step...
        while(true) {
            

            // d_time generating from 1 to 20 mins...
            // TODO d_time depends on fluxDensity - more complex
            int d_time = NormalRandom(meanTime,stddev);
            d_time = intervalCram(d_time);


            cout << "gen d_time: " << d_time << endl;

            stepTime += d_time;
            TimeTick(d_time);
            if (stepTime > modellingStep) {
                break;
            }

            Request req = generateNewReq();

            if (gasStation.ServiceReq(req)) {
                req.status = Accepted;
            }
            else {
                req.status = Declined;
            }
            ReqList.push_back(req); // adding new request to a list
            //cout << "generated new Req for column " << req.colNum << " !" << endl;
            /*
            for (Request r : ReqList) {
                cout << "status: " << r.status << " col: "<< r.colNum << endl;
            }
            */
            gasStation.UpdateReqsTime(d_time); // only the head of queue
            gasStation.ClearServicedReqs(); // pop the head of queue if service time's over
        }

        gasStation.UpdateReqsTime(stepTime-modellingStep);

        gasStation.ClearServicedReqs();


        TimeTick(stepTime-modellingStep);
    }

    void TimeTick(int t_mins) { // updating the modelling time
        currTime[1] += t_mins;
        if (currTime[1] > minsInDay) {
            currTime[0] += 1;
            currTime[1] -= minsInDay;
        }
    };

    
    bool isWeekEnd() {
        if (currTime[0] > 4) {
            return true;
        }
        return false;
    }
};
