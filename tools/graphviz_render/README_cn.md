第一步：安装python3及以上

第二步：安装依赖
  `pip3 install -r requirements.txt`

第三步：运行工具
  `python3 ./src/main.py /tmp/gviz`   （备注： /tmp/gviz是管道名，可自定义）
  如果运行报以下错误
  ![](images/image.png)
  
  请安装：
  `sudo apt-get install libxcb-cursor0 libxcb-xinerama0 libxcb-randr0 libxcb-util0-dev`

第四步：往管道文件写数据
  示例：`echo "diagraph{ A->B; B->C; A->C }" > /tmp/gviz`
