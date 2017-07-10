# 缓存设计思路
![image](http://note.youdao.com/yws/public/resource/06d1627f4bdbd1b9403cd03c943ba6b8/xmlnote/FDA1867E566646B9AF38EF8DB8E76931/373)
## 优化目标：
    使用缓存来减少后端web服务器的压力
    
## 二级缓存
    Post请求：
    在经过请求处理模块之后，直接转发给后端的web server。
    Get请求：
    1. 查看本地内存是否存在请求的内容，若存在，返回给客户端，若不存在，执行2。
    2. 去redis中查询请求的内容。若redis中也不存在，执行3。
    3. 将消息转发给后端web server。获得响应后，更新缓存内容。
    
### 缓存置换策略
    本地缓存：严格限制容量，使用LRU、LFU等策略。
    redis缓存:为key设置合理的ttl。



