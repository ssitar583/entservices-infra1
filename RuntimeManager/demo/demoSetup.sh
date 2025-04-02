#!/bin/sh
chmod 777 /opt/demo/emptyAppService.sh
cp /opt/demo/youtubeDobbySpec /tmp/specchange
cp -R /opt/demo/youtube /opt/.
mount --bind /opt/demo/sky-appsservice.service /etc/systemd/system/sky.service.wants/sky-appsservice.service
#mount --bind /opt/demo/wpeframework.service /etc/systemd/system/multi-user.target.wants/wpeframework.service
systemctl daemon-reload
systemctl restart sky-appsservice
systemctl restart wpeframework
