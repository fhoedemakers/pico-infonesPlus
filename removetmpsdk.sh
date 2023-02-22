if [ "$RETAINSDK" == "" -a -d /tmp/$$ ] ; then
	echo "Removing tmp SDK"
	rm -rf /tmp/$$
fi
