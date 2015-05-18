#!/bin/bash
git add *
git commit -m $1
git remote add origin https://github.com/gaodong0537/huawei2015.git  ## 即刚刚创建的仓库的地址
git push -u origin huawei_lishi	##推送代码到远程代码库
