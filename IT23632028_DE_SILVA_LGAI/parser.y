%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // ADDED for isdigit()

FILE *out;
void yyerror(const char *s);
int yylex(void);

/* Variable table */
#define MAX_VARS 256
static char *var_name[MAX_VARS];
static char *var_value[MAX_VARS];
static int var_count = 0;

/* Config storage */
static char *conf_base = NULL;
#define MAX_CONF_HDRS 64
static char *conf_hdr_k[MAX_CONF_HDRS];
static char *conf_hdr_v[MAX_CONF_HDRS];
static int conf_hdr_count = 0;

extern int yylineno;

// ADDED: Error tracking
static int error_occurred = 0;

// ADDED: Semantic error checking functions (return 1 for error, 0 for OK)
static int check_identifier(const char *ident) {
    if (ident == NULL || ident[0] == '\0') return 0;
    
    // Check if identifier starts with digit
    if (isdigit(ident[0])) {
        fprintf(stderr, "Line %d: expected IDENT after 'let'\n", yylineno);
        error_occurred = 1;
        return 1;
    }
    return 0;
}

static int check_body_type(int is_string) {
    if (!is_string) {
        fprintf(stderr, "Line %d: expected STRING after 'body ='\n", yylineno);
        error_occurred = 1;
        return 1;
    }
    return 0;
}

static int check_status_type(int is_number) {
    if (!is_number) {
        fprintf(stderr, "Line %d: expected NUMBER for status\n", yylineno);
        error_occurred = 1;
        return 1;
    }
    return 0;
}

static void add_var(const char *n, const char *v) {
    if (var_count >= MAX_VARS) return;
    var_name[var_count] = strdup(n);
    var_value[var_count] = strdup(v);
    var_count++;
}
static const char *lookup_var(const char *n) {
    for (int i=0;i<var_count;i++) if (strcmp(var_name[i], n)==0) return var_value[i];
    return "";
}

static char *unquote(const char *s) {
    size_t L = strlen(s);
    if (L>=2 && s[0]=='"' && s[L-1]=='"') {
        char *r = malloc(L-1);
        memcpy(r, s+1, L-2);
        r[L-2]=0;
        return r;
    }
    return strdup(s);
}

/* substitute $name inside inner (unquoted) string, return newly allocated string (unquoted) */
static char *subst_inner(const char *inner) {
    size_t cap = strlen(inner) + 128;
    char *out = malloc(cap);
    out[0]=0;
    const char *p = inner;
    while (*p) {
        if (*p == '$') {
            p++;
            char id[128]; int ii=0;
            while ((*p>='A' && *p<='Z') || (*p>='a' && *p<='z') || (*p>='0' && *p<='9') || *p=='_') {
                if (ii < (int)sizeof(id)-1) id[ii++]=*p;
                p++;
            }
            id[ii]=0;
            const char *val = lookup_var(id);
            size_t need = strlen(out) + strlen(val) + 1;
            if (need >= cap) { cap = need + 128; out = realloc(out, cap); }
            strcat(out, val);
        } else {
            size_t l = strlen(out);
            if (l+2 >= cap) { cap = cap + 128; out = realloc(out, cap); }
            out[l]=*p; out[l+1]=0;
            p++;
        }
    }
    return out;
}

/* take a quoted STRING token, substitute vars, and return new quoted string (including quotes) */
static char *subst_quoted(const char *quoted) {
    char *inner = unquote(quoted);
    char *s = subst_inner(inner);
    free(inner);
    size_t total = strlen(s) + 3;
    char *res = malloc(total);
    snprintf(res, total, "\"%s\"", s);
    free(s);
    return res;
}

/* generate header of file */
static void emit_header(void) {
    fprintf(out,
        "import org.junit.jupiter.api.*;\n"
        "import static org.junit.jupiter.api.Assertions.*;\n"
        "import java.net.http.*; import java.net.*; import java.time.Duration;\n"
        "import java.nio.charset.StandardCharsets; import java.util.*;\n\n"
        "public class GeneratedTests {\n"
        "  static String BASE = \"http://localhost:8080\";\n"  
        "  static Map<String,String> DEFAULT_HEADERS = new HashMap<>();\n"  
        "  static HttpClient client;\n\n"  
    );
}

/* Emit the single @BeforeAll setup block (writes DEFAULT_HEADERS and optionally BASE) */
static void emit_setup(void) {
    fprintf(out, "  @BeforeAll\n");
    fprintf(out, "  static void setup() {\n");
    fprintf(out, "    client = HttpClient.newBuilder().connectTimeout(Duration.ofSeconds(5)).build();\n");
    if (conf_hdr_count==0) {
        fprintf(out, "    DEFAULT_HEADERS.put(\"Content-Type\",\"application/json\");\n");
    } else {
        for (int i=0;i<conf_hdr_count;i++) {
            fprintf(out, "    DEFAULT_HEADERS.put(\"%s\",\"%s\");\n", conf_hdr_k[i], conf_hdr_v[i]);
        }
    }
    if (conf_base) {
        fprintf(out, "    BASE = %s;\n", conf_base);
    }
    fprintf(out, "  }\n\n");
}


%}

%union {
    char *str;
    int num;
}

%token CONFIG BASE_URL HEADER LET TEST GET POST PUT DELETE EXPECT STATUS BODY CONTAINS DESCRIPTION
%token <str> IDENT STRING
%token <num> NUMBER

%type <str> config_items config_item
%type <str> statements statement request_stmt request_items request_item expect_stmt description_stmt

%%

program:
      prologue sections epilogue
    ;

prologue:
      { out = fopen("GeneratedTests.java", "w"); emit_header(); emit_setup(); }
    ;

epilogue:
      { fprintf(out, "}\n"); fclose(out); }
    ;

sections:
      /* empty */
    | sections section
    ;

section:
      config_block
    | var_decl
    | test_block
    ;

config_block:
      CONFIG '{' config_items '}' { free($3); }
    ;

config_items:
      /* empty */ { $$ = strdup(""); }
    | config_items config_item {
          size_t L = strlen($1)+strlen($2)+1;
          $$ = malloc(L+1);
          strcpy($$, $1); strcat($$, $2);
          free($1); free($2);
      }
    ;

config_item:
      BASE_URL '=' STRING ';' {
          /* store quoted base url token (keep quotes) */
          if (conf_base) free(conf_base);
          conf_base = strdup($3);
          $$ = strdup("");
          free($3);
      }
    | HEADER STRING '=' STRING ';' {
          char *k = unquote($2);
          char *v = unquote($4);
          if (conf_hdr_count < MAX_CONF_HDRS) {
              conf_hdr_k[conf_hdr_count] = strdup(k);
              conf_hdr_v[conf_hdr_count] = strdup(v);
              conf_hdr_count++;
          }
          free(k); free(v);
          $$ = strdup("");
          free($2); free($4);
      }
    ;

var_decl:
      LET IDENT '=' STRING ';' {
          if (check_identifier($2)) YYABORT;
          char *val = unquote($4);
          add_var($2, val);
          free(val); free($2); free($4);
      }
    | LET IDENT '=' NUMBER ';' {
          if (check_identifier($2)) YYABORT;
          char tmp[32]; snprintf(tmp, sizeof(tmp), "%d", $4);
          add_var($2, tmp);
          free($2);
      }
    | LET NUMBER IDENT '=' STRING ';' {
          fprintf(stderr, "Line %d: expected IDENT after 'let'\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    | LET NUMBER IDENT '=' NUMBER ';' {
          fprintf(stderr, "Line %d: expected IDENT after 'let'\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    | LET NUMBER '=' STRING ';' {
          fprintf(stderr, "Line %d: expected IDENT after 'let'\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    | LET NUMBER '=' NUMBER ';' {
          fprintf(stderr, "Line %d: expected IDENT after 'let'\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    ;

test_block:
      TEST IDENT '{' statements '}' {
          fprintf(out, "  @Test\n");
          fprintf(out, "  void test_%s() throws Exception {\n", $2);
          fprintf(out, "%s", $4);
          fprintf(out, "  }\n\n");

          free($2);
          free($4);
      }
;
statements:
      /* empty */ { $$ = strdup(""); }
    | statements statement {
          size_t L = strlen($1)+strlen($2)+1;
          $$ = malloc(L+1);
          strcpy($$, $1); strcat($$, $2);
          free($1); free($2);
      }
    ;


statement:
      request_stmt { $$ = $1; }
    | expect_stmt  { $$ = $1; }
    | description_stmt { $$ = $1; }
    ;



request_stmt:
      GET STRING ';' {
          char *url = subst_quoted($2);
          char buf[4096];
          snprintf(buf, sizeof(buf),
    "    HttpRequest.Builder b = HttpRequest.newBuilder(URI.create(BASE + %s))\n"
    "      .timeout(Duration.ofSeconds(10))\n"
    "      .GET();\n"
    "    for (var e: DEFAULT_HEADERS.entrySet()) b.header(e.getKey(), e.getValue());\n"
    "    HttpResponse<String> resp = client.send(b.build(), HttpResponse.BodyHandlers.ofString(StandardCharsets.UTF_8));\n\n",  
    url
);
          $$ = strdup(buf);
          free(url); free($2);
      }
    | GET STRING {
          fprintf(stderr, "Line %d: expected ';' after request\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    | DELETE STRING ';' {
          char *url = subst_quoted($2);
          char buf[4096];
          snprintf(buf, sizeof(buf),
    "    HttpRequest.Builder b = HttpRequest.newBuilder(URI.create(BASE + %s))\n"
    "      .timeout(Duration.ofSeconds(10))\n"
    "      .DELETE();\n"
    "    for (var e: DEFAULT_HEADERS.entrySet()) b.header(e.getKey(), e.getValue());\n"
    "    HttpResponse<String> resp = client.send(b.build(), HttpResponse.BodyHandlers.ofString(StandardCharsets.UTF_8));\n\n",
    url
);
          $$ = strdup(buf);
          free(url); free($2);
      }
    | DELETE STRING {
          fprintf(stderr, "Line %d: expected ';' after request\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
   | POST STRING '[' request_items ']' ';' {
    char *url = subst_quoted($2);
    


    // Separate body and headers
    char *body_code = "HttpRequest.BodyPublishers.noBody()";
    char header_code[4096] = "";
    
    char *items = $4;
    char *body_marker = strstr(items, "BODY_MARKER:");
    if (body_marker) {
        // Extract body code
        body_code = body_marker + 12; // Skip "BODY_MARKER:"
        
        // Extract headers (everything before body marker)
        char *body_start = body_marker;
        *body_start = '\0'; // Terminate headers part
        strncpy(header_code, items, sizeof(header_code)-1);
    } else {
        strncpy(header_code, items, sizeof(header_code)-1);
    }
    
    char buf[8192];
    snprintf(buf, sizeof(buf),
        "    HttpRequest.Builder b = HttpRequest.newBuilder(URI.create(BASE + %s))\n"
        "        .timeout(Duration.ofSeconds(10))\n"
        "        .POST(%s);\n"
        "%s"  // header code
        "    for (var e: DEFAULT_HEADERS.entrySet()) b.header(e.getKey(), e.getValue());\n"
        "    HttpResponse<String> resp = client.send(b.build(), HttpResponse.BodyHandlers.ofString(StandardCharsets.UTF_8));\n\n",
        url, body_code, header_code
    );
    
    $$ = strdup(buf);
    free(url); free($2); free($4);
}
    | POST STRING '[' request_items ']' {
           
          fprintf(stderr, "Line %d: expected ';' after request\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
   | PUT STRING '{' request_items '}' ';' {
    char *url = subst_quoted($2);

    // Separate headers and body
    char *body_code = "HttpRequest.BodyPublishers.noBody()";
    char header_code[4096] = "";

    char *items = $4;
    char *body_marker = strstr(items, "BODY_MARKER:");

    if (body_marker) {
        // Extract body part after marker
        body_code = strdup(body_marker + strlen("BODY_MARKER:"));

        // Truncate items at marker → keeps only header code
        *body_marker = '\0';
        strncpy(header_code, items, sizeof(header_code) - 1);
    } else {
        strncpy(header_code, items, sizeof(header_code) - 1);
    }

    char buf[8192];
    snprintf(buf, sizeof(buf),
        "    HttpRequest.Builder b = HttpRequest.newBuilder(URI.create(BASE + %s))\n"
        "        .timeout(Duration.ofSeconds(10))\n"
        "        .PUT(%s);\n"
        "    for (var e : DEFAULT_HEADERS.entrySet()) b.header(e.getKey(), e.getValue());\n"
        "%s" // request-level headers appended *after* default headers
        "    HttpResponse<String> resp = client.send(b.build(), HttpResponse.BodyHandlers.ofString(StandardCharsets.UTF_8));\n\n",
        url, body_code, header_code
    );

    $$ = strdup(buf);
    free(url);
    free($2);
    free($4);
    free(body_code);
}

    | PUT STRING '{' request_items '}' {
          fprintf(stderr, "Line %d: expected ';' after request\n", yylineno);
          error_occurred = 1;
          YYABORT;
      }
    ;

request_items:
      /* empty */ { $$ = strdup(""); }
    | request_items request_item {
          size_t L = strlen($1)+strlen($2)+1;
          $$ = malloc(L+1);
          strcpy($$, $1); strcat($$, $2);
          free($1); free($2);
      }


    ;

request_item:
      HEADER STRING '=' STRING ';' {
          char *kq = subst_quoted($2);
          char *vq = subst_quoted($4);
          char buf[512];
          snprintf(buf, sizeof(buf), "    b.header(%s, %s);\n", kq, vq);
          $$ = strdup(buf);
          free(kq); free(vq); free($2); free($4);
      }
    | BODY '=' STRING ';' {
          if (check_body_type(1)) YYABORT;
          char *bodyq = subst_quoted($3);
          char buf[1024];
          // Mark this section as body content
          snprintf(buf, sizeof(buf), "BODY_MARKER:HttpRequest.BodyPublishers.ofString(%s)", bodyq);
          $$ = strdup(buf);
          free(bodyq); free($3);
      }
    | BODY '=' NUMBER ';' {
          if (check_body_type(0)) YYABORT;
          $$ = strdup("");
      }
        | description_stmt

;
description_stmt:
      DESCRIPTION STRING
      {
          char buf[1024];
          snprintf(buf, sizeof(buf), "// DESCRIPTION: %s\n", $2);
          $$ = strdup(buf);
          free($2);
      }
;


expect_stmt:
      EXPECT STATUS '=' NUMBER ';' {
          if (check_status_type(1)) YYABORT;
          char buf[128];
          snprintf(buf, sizeof(buf), "    assertEquals(%d, resp.statusCode());\n", $4);
          $$ = strdup(buf);
      }
    | EXPECT STATUS '=' STRING ';' {
          if (check_status_type(0)) YYABORT;
          $$ = strdup(""); // dummy return
      }
    | EXPECT BODY CONTAINS STRING ';' {
          char *s = subst_quoted($4);
          char buf[512];
          snprintf(buf, sizeof(buf), "    assertTrue(resp.body().contains(%s));\n", s);
          $$ = strdup(buf);
          free(s); free($4);
      }
    | EXPECT HEADER STRING '=' STRING ';' {
          char *k = subst_quoted($3);
          char *v = subst_quoted($5);
          char buf[512];
          snprintf(buf, sizeof(buf),
            "    assertEquals(%s, resp.headers().firstValue(%s).orElse(\"\"));\n",
            v, k
          );
          $$ = strdup(buf);
          free(k); free(v); free($3); free($5);
      }
    | EXPECT HEADER STRING CONTAINS STRING ';' {
          char *k = subst_quoted($3);
          char *v = subst_quoted($5);
          char buf[512];
          snprintf(buf, sizeof(buf),
            "    assertTrue(resp.headers().firstValue(%s).orElse(\"\").contains(%s));\n",
            k, v
          );
          $$ = strdup(buf);
          free(k); free(v); free($3); free($5);
      }
    ;

%%

int main() {
    yyparse();
    return error_occurred;
}

void yyerror(const char *s) {
    // Only show generic syntax error if no custom error has been shown yet
    if (!error_occurred && strstr(s, "syntax error") != NULL) {
        fprintf(stderr, "Line %d: syntax error\n", yylineno);
        error_occurred = 1;
    }
}