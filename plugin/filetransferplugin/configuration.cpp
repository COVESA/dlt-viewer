#include "configuration.h"

using namespace std;

Configuration::Configuration()
{
    setDefault();
}

Configuration::~Configuration()
{

}

QString Configuration::getFlstTag() {return tagFLST;}
QString Configuration::getFldaTag() {return tagFLDA;}
QString Configuration::getFlfiTag() {return tagFLFI;}
QString Configuration::getFlerTag() {return tagFLER;}
QString Configuration::getFlifTag() {return tagFLIF;}
QString Configuration::getFlCtIdTag() {return tagFLCtId;}
QString Configuration::getAutoSavePath() {return AutoSavePath;}

void Configuration::setFlstTag(QString newTag) { tagFLST = newTag; }
void Configuration::setFldaTag(QString newTag) { tagFLDA = newTag; }
void Configuration::setFlfiTag(QString newTag) { tagFLFI = newTag; }
void Configuration::setFlerTag(QString newTag) { tagFLER = newTag; }
void Configuration::setFlifTag(QString newTag) { tagFLIF = newTag; }
void Configuration::setFlCtIdTag(QString newTag) { tagFLCtId = newTag; }
void Configuration::setAutoSavePath(QString newTag) { AutoSavePath = newTag; }

void Configuration::setDefault()
{
    tagFLST = "FLST";
    tagFLDA = "FLDA";
    tagFLFI = "FLFI";
    tagFLER = "FLER";
    tagFLIF = "FLIF";
    tagFLCtId = "FILE";
    AutoSavePath = "";
}
