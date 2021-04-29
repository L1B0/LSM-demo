#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>


#define UserConfigPath "/etc/lsm/user_config"
#define RoleConfigPath "/etc/lsm/role_config"

#define SYSCALL_MKDIR 1
#define SYSCALL_RENAME 2
#define SYSCALL_RMDIR   4

char role_list[3][10] = { "MKDIR\0", "RENAME\0", "RMDIR\0"};

typedef struct user_info{
	char name[50];
	int uid;
	unsigned int role_index;
}ui;
int u_num=0;
ui uu[50];

typedef struct role_info{
	char name[50];
	unsigned int syscall;
}ri;
int r_num=0;
ri rr[50];

int str2int(char *s)
{
	int ret=0;
	for(int i=0;i<strlen(s);i++)
	{
		ret = ret*10 + (s[i]-'0');
	}
	
	//printf("linestart-uid: %d\n",ret);
	return ret;
}

char *int2str(int a)
{
	int i=0;
	char s[20]={0}, *ret;
	
	ret = (char*)malloc(20);

	while(a)
	{
		s[i++] = a%10+'0';
		a = a/10;
	}
	
	for(i=0;i<strlen(s);i++)
		ret[i] = s[strlen(s)-i-1];
	ret[strlen(s)] = '\0';
	
	return ret;
}

void output_file(const char *file)
{
	FILE *in= fopen(file, "r");
        char buf[1024];

        while (fgets(buf, sizeof(buf), in) != NULL)
        {
                printf("%s",buf);
        }

        fclose(in);
}

int calc_syscall(char *p)
{
	int ret=0;
	p = strtok(p, ",");

	while(p)
	{
		//puts(p);
		// role config
		if(strcmp(p,"MKDIR") == 0)
		{
			ret |= SYSCALL_MKDIR;
		}
		else if(strcmp(p,"RMDIR") == 0)
		{
			ret |= SYSCALL_RMDIR;
		}
		else if(strcmp(p,"RENAME") == 0)
		{
			ret |= SYSCALL_RENAME;
		}
		p = strtok(NULL, ",");
	}
	//printf("syscall %d\n",ret);
	return ret;
}

void parse_config(const char *file)
{
	// parse config
	FILE *in= fopen(file, "r");
        char buf[1024];

	int file_flag = 0;
	if(file[9] == 'r')
	{
		file_flag = 1;
		r_num = 0;
	}
	else u_num = 0;
	//printf("file flag: %d\n",file_flag);

        while (fgets(buf, sizeof(buf), in) != NULL)
        {
		//puts(buf);
		
		// init
		rr[r_num].syscall = 0;
		buf[strlen(buf)-2] = 0; // RENAME the char ";"
		//puts(buf);

		// get name
		char *p;
        	p = strtok(buf, ":");       
		//printf("111 %s\n",p);
		if(file_flag)
			strcpy(rr[r_num].name, p);
		else
		{
			int uuid = str2int(p);
			struct passwd *pwd = getpwuid(uuid);

			uu[u_num].uid = uuid;
			strcpy(uu[u_num].name, pwd->pw_name);
		}
		//printf("rr %s\n",rr[r_num].name);	
			
		// get others
		p = strtok(NULL,buf);
		if(file_flag == 1)
		{
			rr[r_num].syscall = calc_syscall(p);
			r_num++;
		}
		else if(file_flag == 0)
		{
			//puts(p);
				
			for(int i=0;i<r_num;i++)
			{
				if(strcmp(p,rr[i].name) == 0)
				{
					//printf("%s %s %d\n",p,rr[i].name,i);
					uu[u_num].role_index = i;
					break;
				}
			}
			u_num++;
		}

        }

        fclose(in);
}

void update_user_config()
{
	FILE *out= fopen(UserConfigPath, "w");

	for(int i=0;i<u_num;i++)
	{
		char buf[1024]={0};
		
		strcpy(buf,int2str(uu[i].uid));
		//puts(buf);
		strcat(buf,":");
		//puts(buf);
		strcat(buf,rr[uu[i].role_index].name);
		//puts(buf);
		strcat(buf,";\n");
		//puts(buf);
		fputs(buf,out);	
	}
        fclose(out);
}

void update_role_config()
{
        FILE *out= fopen(RoleConfigPath, "w");

        for(int i=0;i<u_num;i++)
        {
                char buf[1024]={0},syscall_list[50]={0};
		
                strcpy(buf,rr[i].name);
                strcat(buf,":");
                
		if(rr[i].syscall & SYSCALL_MKDIR) strcat(syscall_list,"MKDIR,");
		if(rr[i].syscall & SYSCALL_RENAME) strcat(syscall_list,"RENAME,");
		if(rr[i].syscall & SYSCALL_RMDIR) strcat(syscall_list,"RMDIR,");
		syscall_list[strlen(syscall_list)-1] = '\0';
		//puts(syscall_list);
		strcat(buf,syscall_list);
                
		strcat(buf,";\n");
                fputs(buf,out);
        }
        fclose(out);
}

void output_user()
{
	//puts("output_user");
	puts("name|uid|gid");
	system("cat /etc/passwd|grep -v nologin|grep -v halt|grep -v shutdown|awk -F\":\" '{ print $1\"|\"$3\"|\"$4 }'|more");
}

void output_lsm_user()
{
	//puts("output_lsm_user");
	
	output_file(UserConfigPath);
}

void output_role()
{
	//puts("output_role");

	output_file(RoleConfigPath);
}

void update_user()
{
	//puts("update_user");
	
	// input username to update
	puts("[current user list]");
	for(int i=0;i<u_num;i++) puts(uu[i].name);
	char cur_user[50]={0};
	printf("Input username: ");
	scanf("%s",cur_user);
	
	// output role list to lsm_user
	puts("[role list]:");
	for(int i=0;i<r_num;i++) printf("%s %d\n",rr[i].name,rr[i].syscall);
	
	// input new role
	char new_role[50]={0};
	printf("Input the new role: ");
	scanf("%s",new_role);
	
	// check new role
	int flag = 0, new_role_index=-1;
	for(int i=0;i<r_num;i++)
	{
		if(strcmp(new_role,rr[i].name) == 0)
		{
			flag = 1;
			new_role_index = i;
			break;
		}
        }
	if(flag == 0)
	{
		puts("This role isn't exist!");
		return ;
	}
	
	// check and update user
	flag = 0;
	for(int i=0;i<u_num;i++)
	{
		if(strcmp(cur_user,uu[i].name) == 0)
		{
			flag = 1;
			//printf("find new role index: %d\n",new_role_index);
			uu[i].role_index = new_role_index;
			break;
		}
	}
	if(flag == 0)
        {
                puts("This user isn't exist!");
                return ;
        }
	
	//output_lsm_user();
	update_user_config();
}

void update_role()
{

	printf("Input the role name and auth like 'fish1 RENAME,RMDIR': ");
	char cur_role[50],new_auth[50];
	scanf("%s %s",cur_role,new_auth);

	for(int i=0;i<r_num;i++)
	{
		if(strcmp(cur_role,rr[i].name) == 0)
		{
			rr[i].syscall = calc_syscall(new_auth);
		}
	}

	update_role_config();
}

void delete_info(const char* file)
{
	puts("delete_info");
	
	printf("Input the name: ");
	char del_info[1024];
	scanf("%s",del_info);

	FILE *in= fopen(file, "r");

	int i=0, del_index=-1;
	char buf[50][1024]={0};
	
	while (fgets(buf[i], sizeof(buf[i]), in) != NULL)
	{
		//puts(buf[i]);
		char t[1024];
		strcpy(t,buf[i]);
		strtok(t, ":");
		if(strcmp(t,del_info) == 0)
			del_index = i;
		i++;
	}
	
	fclose(in);
	
	FILE *out = fopen(file, "w");
	for(int j=0;j<i;j++)
	{
		//printf("%s %d\n",buf[j],del_index);
		if(j != del_index)
			fputs(buf[j], out);
	}
	fclose(out);
}

void delete_role()
{
	delete_info(RoleConfigPath);
}

void delete_user()
{
	delete_info(UserConfigPath);
}

void add_role()
{
	puts("add_role");
	
	printf("Input role and authority like 'fish1:RENAME;':\n");
	char ra[1024];
	scanf("%s",ra);

	FILE *in= fopen(RoleConfigPath, "a");
	strcat(ra,"\n");
	fputs(ra, in);
	fclose(in);
}

void add_user()
{
        puts("add_user");

        printf("Input userid and role like '1001:fish1;':\n");
        char ra[1024];
        scanf("%s",ra);

        FILE *in= fopen(UserConfigPath, "a");
        strcat(ra,"\n");
        fputs(ra, in);
        fclose(in);
}


int main()
{
	puts("Welcome to lsm module.\nPlease input your choice:)");
	puts("[0]: exit");
	puts("[1]: output user info");
	puts("[2]: output lsm user info");
	puts("[3]: update user info");
	puts("[4]: delete user");
	puts("[5]: add user");
	puts("[6]: output role info");
	puts("[7]: update role info");
	puts("[8]: delete role");
	puts("[9]: add role");

	int choice=0;
	while(1)
	{
		printf("Choice: ");
		scanf("%d",&choice);
		
		if(choice == 0)
		{
			puts("Bye~");
			break;
		}
		
		parse_config(RoleConfigPath);
		parse_config(UserConfigPath);
		switch (choice){
			case 1:
				output_user();
				break;
			case 2:
				output_lsm_user();
				break;
			case 3: 
				update_user();
				break;
			case 4: 
				delete_user();
				break;
			case 5: 
				add_user();
				break;
			case 6:
				output_role();
				break;
			case 7:
				update_role();
				break;
			case 8:
				delete_role();
				break;
			case 9:
				add_role();
				break;
			default:
				break;
		}
	}
	return 0;
}
