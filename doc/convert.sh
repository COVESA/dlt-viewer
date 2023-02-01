#generate a pdf out of the dlt_viewer_user_manual.txt
#sed '/image/ s/width=\"100%\"/max-width=\"100%\"/' dlt_viewer_user_manual.txt > dlt_viewer_user_manual_4pdf.txt; cp dlt_viewer_user_manual-docinfo.xml dlt_viewer_user_manual_4pdf-docinfo.xml; asciidoc -a docinfo -a lang=en -v -b docbook -d book dlt_viewer_user_manual_4pdf.txt ;   dblatex -V -T db2latex dlt_viewer_user_manual_4pdf.xml

sed '/image/ s/width=\"100%\"/max-width=\"100%\"/' dlt_viewer_plugins_programming_guide.txt > dlt_viewer_plugins_programming_guide_4pdf.txt;asciidoc -a lang=en -v -b docbook -d book dlt_viewer_plugins_programming_guide_4pdf.txt ;   dblatex -V -T db2latex dlt_viewer_plugins_programming_guide_4pdf.xml
#rm dlt_viewer_user_manual_4pdf.txt dlt_viewer_user_manual_4pdf.xml dlt_viewer_plugins_programming_guide_4pdf.txt dlt_viewer_plugins_programming_guide_4pdf.xml  dlt_viewer_user_manual_4pdf-docinfo.xml
#mv dlt_viewer_user_manual_4pdf.pdf dlt_viewer_user_manual.pdf
mv dlt_viewer_plugins_programming_guide_4pdf.pdf dlt_viewer_plugins_programming_guide.pdf
rm -f dlt_viewer_plugins_programming_guide_4pdf.txt dlt_viewer_plugins_programming_guide_4pdf.xml
