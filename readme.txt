
1. 입력 루프를 돌면서 개행문자 없는 스트링으로 입력 획득

// 화면에서 입력받을 때(공백도 입력으로), 빈입력이면 종료,
while ( fgets(buffer, sizeof(buffer), stdin ) != NULL ) {
  if ( sscanf(buffer, "%[^\n]s", buffer) == EOF ) break; // 

}

// 파일에서 입력받을 때(공백도 입력으로), 빈라인을 만나면 아무것도 하지 않고, 파일 끝에서 종료
while ( fgets(buffer, sizeof(buffer), fp ) != NULL ) {
  if ( sscanf(buffer, "%[^\n]s", buffer) == EOF ) continue; // empty line

}

2. 단순 입력값 추출


3. Tokenization 포함




///

sql fiddle

// threaded socket
https://unipro.tistory.com/151

// socket example
https://stackoverflow.com/questions/9513327/gio-socket-server-client-example

https://edgar.tistory.com/entry/Glib-라이브러리

I.타입 정의
  1.타이핑: guint, guchar
  2.이식성: guint8, guint16, guint32, (guint64 if G_HAVE_INT64 is true)
  3.일관성: gpointer == void *, gboolean == int
  
II.메모리 할당
  1. g_malloc
  2. g_free
  3. g_new
  4. g_new0: 0으로 채움
  
III.유틸리티 함수
  1. 대소문자 변환: g_strup, g_strdown
  2. 공백제거: g_strchug(앞), g_strchomp(뒤), g_strip(양쪽)
  3. 할당: g_strdup(할당-->복사-->리턴), g_strndup(개수복사), g_strconcat(할당-->병합-->리턴, 마지막 인수 NULL로 받음) -->g_free 호출필요
  4. 버퍼오버플로우 방지 출력: g_snprintf
  
IV.컨테이너
  0.포인터와 정수형 전환 매크로: GINT_TO_POINTER, GPOINTER_TO_INT
  1.GList: 링크드 리스트
    1.1.기본함수: GList, g_list_append, li->next, li->data, g_list_free, g_list_foreach + g_free 결합
                g_list_nth_data, g_list_index, g_list_find_custom, g_list_sort, g_list_copy(데이터 복사 안함)
                g_list_first, g_list_last, g_list_reverse
                GFunc, GCompareFunc
  2.GString: 가변 문자열
    2.1.생성/해제: g_string_new, g_string_free(TRUE: 완전히 해제, FALSE: 구조체만 해제)
    2.2.변경: g_string_append, g_string_append_c, g_string_prepend, g_string_prepend_c, g_string_insert, g_string_insert_c
             g_string_erase
    2.3.출력: g_string_sprintf(덮어쓰기), g_string_sprintfa(뒤에 추가)
    2.4.변경: g_string_up, g_string_down, g_string_truncate, g_string_assign
