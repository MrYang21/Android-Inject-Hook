#include <iostream>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

int main() {
    const char *command = "pm path com.tencent.ig";  // 获取 APK 路径的命令
    char buffer[128];
    
    FILE *fp = popen(command, "r");  // 执行命令并获取输出
    if (fp == nullptr) {
        std::cerr << "Failed to run command" << std::endl;
        return 1;
    }

    // 读取命令输出
    if (fgets(buffer, sizeof(buffer), fp) == nullptr) {
        std::cerr << "Failed to get output from command" << std::endl;
        fclose(fp);
        return 1;
    }

    // 去掉换行符
    buffer[strcspn(buffer, "\n")] = 0;

    // 检查输出是否包含 "package:"，并删除它
    std::string apkPath(buffer);
    if (apkPath.find("package:") == 0) {
        apkPath.erase(0, 8);  // 删除 "package:"
    }

    // 检查输出是否包含 "base.apk" 并删除它
    size_t pos = apkPath.find("base.apk");
    if (pos != std::string::npos) {
        apkPath.erase(pos);  // 删除 "base.apk"
    }

    // 加上 "lib/arm64/"
    apkPath += "lib/arm64/";

    std::cout << "Target path: " << apkPath << std::endl;  // 输出最终的路径

    fclose(fp);

    // 获取当前目录下的 "peizhi" 文件夹
    std::string sourceDir = "./peizhi";
    if (!fs::exists(sourceDir)) {
        std::cerr << "Source directory 'peizhi' does not exist." << std::endl;
        return 1;
    }

    // 赋予 "peizhi" 文件夹里的文件 777 权限
    for (const auto &entry : fs::directory_iterator(sourceDir)) {
        try {
            fs::permissions(entry.path(), fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all);  // 赋予 777 权限
            std::cout << "Granted 777 permission to " << entry.path() << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "Error setting permissions: " << e.what() << std::endl;
        }
    }
    
    // 复制文件到目标路径
    for (const auto &entry : fs::directory_iterator(sourceDir)) {
        try {
            std::string targetPath = apkPath + entry.path().filename().string();  // 拼接目标路径
            std::cout << "Copying " << entry.path() << " to " << targetPath << std::endl;
            fs::copy(entry.path(), targetPath, fs::copy_options::overwrite_existing);  // 复制并覆盖
        } catch (const std::exception &e) {
            std::cerr << "Error copying file: " << e.what() << std::endl;
        }
    }

    // 变量提前声明，避免重复定义问题
    int ret;

    // 启动应用
    const char *startAppCommand = "am start com.tencent.ig/com.epicgames.ue4.SplashActivity";
    ret = system(startAppCommand);  // 使用 system 函数启动应用
    if (ret != 0) {
        std::cerr << "Failed to start app." << std::endl;
        return 1;
    }

    // 等待 2 秒
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 拼接 .so 文件的完整路径
    std::string libAnjectPath = apkPath + "libAnject.so";
    std::string libRefusePath = apkPath + "libRefuse.so";
    std::string libDobbyPath = apkPath + "libdobby.so";  // 新增 libdobby.so 路径

    // 第一次执行 libAnject.so，注入 libdobby.so
    //std::cout << "Injecting libdobby.so with libAnject.so..." << std::endl;
    //std::string injectDobbyCommand = "LD_LIBRARY_PATH=" + apkPath + " " + libAnjectPath + 
    //                                 " -n com.tencent.igce -so " + libDobbyPath;
    //ret = system(injectDobbyCommand.c_str());  // 执行注入 libdobby.so
    //if (ret != 0) {
    //    std::cerr << "Failed to inject libdobby.so." << std::endl;
    //    return 1;
    //}

    // 第二次执行 libAnject.so，注入 libRefuse.so
    std::cout << "Injecting libRefuse.so with libAnject.so..." << std::endl;
    std::string injectRefuseCommand = "LD_LIBRARY_PATH=" + apkPath + " " + libAnjectPath + 
                                      " -n com.tencent.igce -so " + libRefusePath;
    ret = system(injectRefuseCommand.c_str());  // 执行注入 libRefuse.so
    if (ret != 0) {
        std::cerr << "Failed to inject libRefuse.so." << std::endl;
        return 1;
    }

    return 0;
}
