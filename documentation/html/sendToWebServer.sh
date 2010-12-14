
# doesn't overwrite newer files on server 
# (so private PHP settings that aren't stored in Mercurial are preserved)

rsync -avzu -e ssh . jcr13@northcountrynotes.org:www/insideastarfilledskyRoot/