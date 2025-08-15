#ifndef CSVPARSER_H
#define CSVPARSER_H
#include<QVector>
#include<QString>
#include<QFile>

//文件记录结构体
struct DefectRecord
{
    int nFileIndex;     //文件索引
    int nDefectIndex;   //缺陷索引
    int nDefectLevel;   //缺陷等级
    float fCentreX;     //缺陷中心x坐标
    float fCentreY;     //缺陷中心y坐标
    float fLength;      //缺陷长度
    float fHeight;      //缺陷高度
};

//csv文件解析类
class CsvParser
{
public:
    static QVector<DefectRecord> ParseCsv(QString strCsvPath);  //解析 CSV 文件，返回缺陷记录列表
};

#endif // CSVPARSER_H
