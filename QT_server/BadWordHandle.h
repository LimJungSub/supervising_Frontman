#ifndef BADWORDHANDLE_H
#define BADWORDHANDLE_H

#include <QObject>
#include <vector>
#include <fstream>
#include <algorithm>
#include <QDebug>

using namespace std;

class BadWordHandle : public QObject {
    Q_OBJECT

signals:
    void badWordsUpdated();  // QML에서 데이터 변경을 감지할 수 있도록 하는 시그널

private:
    string filePath;  // 비속어 목록이 저장된 파일 경로
    static vector<string> badWords;  // 비속어 목록을 저장하는 컨테이너

    // 비속어 목록을 파일에서 불러오는 함수
    void loadBadWords() {
        ifstream file(filePath);
        if (!file.is_open()) {
            qDebug() << "파일을 열 수 없습니다:" << QString::fromStdString(filePath);
            return;
        }

        badWords.clear();  // 기존 목록을 초기화

        string line;
        while (getline(file, line)) {  // 한 줄씩 읽기
            if (!line.empty()) {
                badWords.push_back(line);
            }
        }
        file.close();
        qDebug() << "비속어 목록 로드 완료, 총" << badWords.size() << "개 항목";
    }

    // 비속어 목록을 파일에 저장하는 함수
    void saveBadWords() {
        ofstream file(filePath);
        if (!file.is_open()) {
            qDebug() << "파일을 저장할 수 없습니다:" << QString::fromStdString(filePath);
            return;
        }

        for (const auto& word : badWords) {
            file << word << '\n';  // 각 단어를 줄바꿈과 함께 저장
        }
        file.close();
        qDebug() << "비속어 목록 저장 완료";
        emit badWordsUpdated();  // 데이터 변경 시 QML에 알림
    }


public:
    explicit BadWordHandle(const QString& filePath, QObject* parent = nullptr)
        : QObject(parent), filePath(filePath.toStdString()) { //이니셜라이저로 filePath초기화헀다.
        loadBadWords();  // 프로그램 시작 시 비속어 목록 로드
    }

    // QML에서 호출할 수 있는 함수 (Q_INVOKABLE)
    Q_INVOKABLE QStringList getBadWords(){
        QStringList list;
        for (auto& word : badWords) {
            list.append(QString::fromStdString(word));  // C++의 string을 QML에서 사용 가능하게 변환
        }
        return list;
    }

    Q_INVOKABLE void addBadWord(QString& word) {
        string strWord = word.toStdString();

        // 중복 체크: 리스트에 존재하지 않는 경우만 추가
        if (find(badWords.begin(), badWords.end(), strWord) == badWords.end()) {
            badWords.push_back(strWord);
            saveBadWords();
            qDebug() << "새 비속어 추가됨:" << QString::fromStdString(strWord);
        } else {
            qDebug() << "이미 존재하는 비속어:" << QString::fromStdString(strWord);
        }
    }

    Q_INVOKABLE void removeBadWord(QString& word) {
        string strWord = word.toStdString();

        // 벡터에서 해당 단어 제거 (remove() + erase() 사용)
        // remove()는 삭제된 이후 새로운 끝 위치를 반환함 -> 그래서 erase()를 사용해야 완전히 제거됨.
        auto it = remove(badWords.begin(), badWords.end(), strWord);
        if (it != badWords.end()) {
            badWords.erase(it, badWords.end());
            saveBadWords();  // 파일 저장
            qDebug() << "비속어 삭제됨:" << QString::fromStdString(strWord);
        } else {
            qDebug() << "삭제하려는 비속어가 존재하지 않음:" << QString::fromStdString(strWord);
        }
    }

    //메시지 내에 비속어가 포함되어 있는지 검사
    static bool isContainBadWord(QString &message){
        for ( auto &word : badWords) {
            // 대소문자 구분 없이 검사합니다.
            if (message.contains(QString::fromStdString(word), Qt::CaseInsensitive)) {
                return true;
            }
        }
        return false;
    }
};

#endif // BADWORDHANDLER_H
