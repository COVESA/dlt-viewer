#include "configuration.h"

using namespace std;

Configuration::Configuration():tagFLST("FLST"),tagFLDA("FLDA"),tagFLFI("FLFI"),tagFLIF("FLIF"),tagFLER("FLER"),tagFLAppId("FLTR"),tagFLCtId("FLTR")
{

}

Configuration::~Configuration()
{

}

QString Configuration::getFlstTag() {return tagFLST;}
QString Configuration::getFldaTag() {return tagFLDA;}
QString Configuration::getFlfiTag() {return tagFLFI;}
QString Configuration::getFlerTag() {return tagFLER;}
QString Configuration::getFlifTag() {return tagFLIF;}
QString Configuration::getFlAppIdTag() {return tagFLAppId;}
QString Configuration::getFlCtIdTag() {return tagFLCtId;}

void Configuration::setFlstTag(QString newTag) { tagFLST = newTag; }
void Configuration::setFldaTag(QString newTag) { tagFLDA = newTag; }
void Configuration::setFlfiTag(QString newTag) { tagFLFI = newTag; }
void Configuration::setFlerTag(QString newTag) { tagFLER = newTag; }
void Configuration::setFlifTag(QString newTag) { tagFLIF = newTag; }
void Configuration::setFlAppIdTag(QString newTag) { tagFLAppId = newTag; }
void Configuration::setFlCtIdTag(QString newTag) { tagFLCtId = newTag; }
