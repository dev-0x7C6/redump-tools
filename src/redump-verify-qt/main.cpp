
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <thread>

#include <cryptopp/sha.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <CLI/CLI.hpp>

#include "checksums.hpp"
#include "common.hpp"
#include "fs.hpp"

#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include <QString>
#include <QTimer>

using namespace std::chrono_literals;

using progress_t = std::atomic<double>;

void qt_generate_progress_dialog(QString &&title, progress_t &progress) {
    QProgressDialog dialog;
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &dialog, [&dialog, &progress]() {
        dialog.setValue(static_cast<int>(progress * 1000.0));
    });

    dialog.setLabelText(title);
    dialog.setMinimum(0);
    dialog.setMaximum(1000);
    dialog.setCancelButton(nullptr);
    timer.start(5ms);
    dialog.show();
    dialog.exec();
}

template <typename return_type>
return_type qt_progress_task(QString &&title, std::function<return_type(progress_t &)> &&callable) {
    progress_t progress{};
    auto task = std::packaged_task<return_type()>([callable{std::move(callable)}, &progress]() {
        return callable(progress);
    });

    auto result = task.get_future();

    std::thread thread(std::move(task));

    qt_generate_progress_dialog(std::move(title), progress);
    result.wait();
    thread.join();

    return result.get();
}

auto main(int argc, char **argv) -> int {
    QApplication qapp(argc, argv);
    CLI::App app("redump-verify-gui");

    std::vector<std::string> paths;
    std::vector<std::string> files;

    app.add_option("-i,--input", paths, "xml redump database") //
        ->allow_extra_args()
        ->check(CLI::ExistingFile);

    app.add_option("-v,--verify", files, "files to verify") //
        ->required()
        ->allow_extra_args()
        ->check(CLI::ExistingFile);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return 1;
    }

    std::filesystem::path db_dir(get_home_dir() / ".cache" / "redump" / "db");
    std::filesystem::create_directories(db_dir);

    for (auto &&entry : std::filesystem::directory_iterator{db_dir})
        if (entry.is_regular_file())
            paths.emplace_back(entry.path());

    redump::games games;

    for (auto &&path : paths) {
        auto db = redump::load(path).value_or(std::vector<redump::game>{});
        std::move(db.begin(), db.end(), std::back_inserter(games));
    }

    std::map<std::string, redump::game> sha1_to_game;

    for (auto &&game : games)
        for (auto &&rom : game.roms)
            sha1_to_game[rom.sha1] = game;

    auto is_valid{true};
    std::atomic<double> progress;

    for (auto &&file : files) {
        const auto hash = qt_progress_task<std::string>(QObject::tr("Checking ") + QString::fromStdString(file), [file](progress_t &progress) {
            return checksum::compute<CryptoPP::SHA1>(file, progress);
        });

        const auto valid = sha1_to_game.contains(hash);
        if (valid)
            QMessageBox::information(nullptr, QObject::tr("Valid backup"), QObject::tr("File %1 is valid backup of %2") //
                                                                               .arg(QString::fromStdString(file), QString::fromStdString(sha1_to_game[hash].name)),
                QMessageBox::Ok);
        else
            QMessageBox::warning(nullptr, QObject::tr("Corrupted file or unknown"), QObject::tr("File %1 is corrupted or not known") //
                                                                                        .arg(QString::fromStdString(file)),
                QMessageBox::Ok);
        is_valid &= valid;
    }

    return is_valid ? 0 : 2;
}
