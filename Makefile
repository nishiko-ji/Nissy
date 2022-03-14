CC 		= g++	# コンパイラ
CFLAGS 	= -Wall  	# コンパイルオプション
TARGET 	= Nissy	# 実行ファイル名
SRCS 	= main.cpp	# コンパイル対象のソースコード
SRCS 	+= game.cpp
SRCS 	+= uct.cpp
SRCS 	+= client.cpp
SRCS 	+= log.cpp
OBJS 	= $(SRCS:.cpp=.o)	# オブジェクトファイル名
INCDIR  = -I../inc	# インクルードファイルのあるディレクトリパス
LIBDIR  = 	# ライブラリのあるディレクトリパス
LIBS    = 	# 追加するライブラリファイル
# ターゲットファイル生成
$(TARGET): $(OBJS)
	$(CC) -pthread -o $@ $^ $(LIBDIR) $(LIBS)
	
# オブジェクトファイル生成
$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(INCDIR) -c $(SRCS)

# "make all"で make cleanとmakeを同時に実施。
all: clean $(OBJS) $(TARGET)
# .oファイル、実行ファイル、.dファイルを削除
clean:
	-rm -f $(OBJS) $(TARGET) *.d

