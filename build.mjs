#!/usr/bin/env node

import fs from 'node:fs';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';

const ROOT = path.dirname(fileURLToPath(import.meta.url));
const CLEAN = process.argv.includes('--clean');

const BIN_FILES = ['SandMan.exe', 'SbieCtrl.exe', 'SbieSvc.exe', 'Start.exe', 'SbieDll.dll'];

const p = (...s) => path.join(ROOT, ...s);
const rel = (s) => path.relative(ROOT, s) || '.';

function step(msg) {
    console.log(`\n=== ${msg} ===`);
}

/** 执行命令，继承 stdio，失败抛异常 */
function run(command, args, cwd = ROOT) {
    console.log(`$ ${command} ${args.join(' ')}`);
    const r = spawnSync(command, args, { cwd, stdio: 'inherit' });
    if (r.error) throw new Error(`无法执行 ${command}：${r.error.message}`);
    if (r.status !== 0) throw new Error(`${command} 失败，退出码 ${r.status}`);
    return r;
}

function exists(relPath) {
    return fs.existsSync(p(relPath));
}

/** 移动/覆盖文件（Windows 上 rename 无法覆盖已存在目标） */
function moveFile(src, dest) {
    fs.mkdirSync(path.dirname(dest), { recursive: true });
    if (fs.existsSync(dest)) fs.rmSync(dest, { force: true });
    fs.renameSync(src, dest);
    console.log(`  移动 ${rel(src)} -> ${rel(dest)}`);
}

function remove(relPath) {
    fs.rmSync(p(relPath), { force: true });
}

// ---------------------------------------------------------------------------
// 1. 环境检查
// ---------------------------------------------------------------------------
function checkToolchain() {
    step('检查编译环境');
    const r = spawnSync('clang', ['-v'], { stdio: 'ignore' });
    if (r.error || r.status !== 0) {
        console.error('检测到您未安装 LLVM-Mingw，或未将其添加到 PATH 环境变量中');
        console.error('本项目需要 LLVM-Mingw 以编译内核驱动程序');
        console.error('可在下方网址下载 LLVM-Mingw（~100MB）');
        console.error('https://github.com/mstorsjo/llvm-mingw/releases');
        throw new Error('缺少 clang');
    }
    console.log('  编译环境 OK');
}

// ---------------------------------------------------------------------------
// 2. 先构建工具链依赖：SbieSign / SbieLoader
// ---------------------------------------------------------------------------
function buildHelpers() {
    step('构建辅助工具');

    if (!exists('SbieSign/SbieSign.exe') || CLEAN) {
        run('clang', [
            'SbieSign.c', '-o', 'SbieSign.exe',
            '-l', 'bcrypt', '-l', 'crypt32',
            '-Wl,-O3,-icf=all,-s', '-O3', '-flto=full',
        ], p('SbieSign'));
        console.log('  SbieSign.exe 构建完成');
    } else {
        console.log('  SbieSign.exe 已存在，跳过');
    }

    if (!exists('bin_loader/lua53-64.dll') || CLEAN) {
        run('clang', [
            'dllmain.cpp',
            '-ffunction-sections', '-fdata-sections',
            '-Wno-invalid-source-encoding', '-shared', '-DUNICODE',
            '-o', 'lua53-64.dll', '-fms-extensions',
            '-Wl,-s,-O3,-icf=all,--gc-sections,--disable-runtime-pseudo-reloc,--entry=DllMain',
            '-O3', '-flto=full',
        ], p('SbieLoader'));
        moveFile(p('SbieLoader', 'lua53-64.dll'), p('bin_loader', 'lua53-64.dll'));
    } else {
        console.log('  lua53-64.dll 已存在，跳过');
    }
}

// ---------------------------------------------------------------------------
// 3. 检查 bin/ 下的原始文件并复制到根目录
// ---------------------------------------------------------------------------
function prepareBinaries() {
    step('检查 bin 目录');
    const missing = BIN_FILES.filter((f) => !exists(path.join('bin', f)));
    if (missing.length) {
        console.error('下列部分或全部文件缺失，请复制到 bin 文件夹');
        BIN_FILES.forEach((f, i) => console.error(`  ${i + 1}. ${f}`));
        console.error(`实际缺失：${missing.join(', ')}`);
        throw new Error('bin 目录文件不全');
    }

    // dll 不需要修改，所以只复制 exe
    for (const f of fs.readdirSync(p('bin')).filter((n) => n.toLowerCase().endsWith('.exe'))) {
        fs.copyFileSync(p('bin', f), p(f));
        console.log(`  复制 bin/${f} -> ${f}`);
    }
}

// ---------------------------------------------------------------------------
// 4. 构建 SbieWipe（直接链接 SbieDll.dll）
// ---------------------------------------------------------------------------
function buildSbieWipe() {
    step('构建 SbieWipe');
    const cwd = p('SbieWipe');

    // lld 可以直接读取 DLL 的导出表并隐式生成导入库，
    // 所以不再需要 Dll2Lib（objdump -> def -> dlltool）这一步。
    run('clang', [
        '-O3', '-flto=full', '-ffunction-sections', '-fdata-sections', '-pipe',
        'SbieWipe.c', '-o', 'SbieWipe.exe',
        '-Wl,-s,-O3,-icf=all,--gc-sections,--disable-runtime-pseudo-reloc,../bin/SbieDll.dll',
    ], cwd);

    moveFile(p('SbieWipe', 'SbieWipe.exe'), p('SbieWipe.exe'));
}

// ---------------------------------------------------------------------------
// 5. 运行签名工具：替换 exe 中的公钥并生成签名
// ---------------------------------------------------------------------------
function signBinaries() {
    step('运行 SbieSign (现在请在文件内修改账户名称喵)');
    const SIGN_USER = 'MiaoAo';
    // 参数格式：<证书名> [待替换公钥的文件...] "" [待签名的文件...]
    const args = [
        SIGN_USER,
        'SbieSvc.exe', 'SandMan.exe', '',
        'Start.exe', 'SbieSvc.exe', 'SbieCtrl.exe', 'SandMan.exe', 'SbieWipe.exe',
    ];

    const r = spawnSync(p('SbieSign', 'SbieSign.exe'), args, {
        cwd: ROOT,
        stdio: ['ignore', 'pipe', 'pipe'],
        maxBuffer: 1 * 1024 * 1024,
    });
    if (r.error) throw new Error(`无法执行 SbieSign.exe：${r.error.message}`);

    const stdout = (r.stdout || Buffer.alloc(0)).toString('utf8');
    const stderr = (r.stderr || Buffer.alloc(0)).toString('utf8');

    // stderr 里是证书内容（NAME/TYPE/SIGNATURE），保留成 Certificate.dat
    fs.writeFileSync(p('Certificate.dat'), r.stderr || Buffer.alloc(0));

    if (r.status !== 0) {
        throw new Error(`SbieSign 失败，退出码 ${r.status}\n${stderr.trim()}`);
    }

    const lines = stdout.split(/\r?\n/);
    const nonEmpty = lines.filter((l) => l.trim().length > 0);
    const lastLine = nonEmpty.length ? nonEmpty[nonEmpty.length - 1].trim() : '';
    if (lastLine !== 'All done') {
        throw new Error(
            '签名失败！\n-----------------------------------\n' + (stdout.trim() || stderr.trim()) +
            '\n-----------------------------------',
        );
    }

    const replaceLine = (lines[0] || '').trim();
    if (!replaceLine.startsWith('static const UCHAR Replace')) {
        throw new Error(`签名工具输出的第一行不是公钥数组：\n${replaceLine.slice(0, 200)}`);
    }
    return replaceLine;
}

// ---------------------------------------------------------------------------
// 6. 生成 SbiePatch.c（占位符替换）
// ---------------------------------------------------------------------------
function generatePatchSource(replaceLine) {
    step('生成 SbiePatch.c');
    const tplPath = p('SbiePatch', 'SbiePatch.c.tpl');
    const outPath = p('SbiePatch', 'SbiePatch.c');
    const tpl = fs.readFileSync(tplPath, 'utf8');
    const out = tpl.replace('PLACEHOLDER', () => replaceLine);
    fs.writeFileSync(outPath, out, 'utf8');
    return outPath;
}

// ---------------------------------------------------------------------------
// 7. 编译内核驱动
// ---------------------------------------------------------------------------
function buildDriver() {
    step('编译 SbiePatch.sys');
    const cwd = p('SbiePatch');
    const cFile = p('SbiePatch', 'SbiePatch.c');

    try {
        run('clang', [
            'SbiePatch.c', '-shared', '-o', 'SbiePatch.sys',
            '-l', 'ntoskrnl',
            '-Wno-invalid-source-encoding', '-fms-extensions',
            '-Wl,-s,--entry=DriverEntry',
            '-nostartfiles', '-nodefaultlibs', '-ffreestanding',
            '-Wl,--disable-dynamicbase,--disable-high-entropy-va,--disable-nxcompat,--disable-runtime-pseudo-reloc',
            '-O3', '@arg_tiny_unsafe.txt',
        ], cwd);
    } catch (e) {
        // 编译失败时保留生成的 .c，方便对照检查
        console.error(`\n编译失败，已保留生成的源码：${rel(cFile)}`);
        throw e;
    }

    remove('SbiePatch/SbiePatch.c');
    moveFile(p('SbiePatch', 'SbiePatch.sys'), p('bin_loader', 'SbiePatch.sys'));
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
function main() {
    if (CLEAN) {
        step('清理中间产物');
        for (const f of [
            'SbiePatch/SbiePatch.c', 'SbiePatch/SbiePatch.sys',
            'SbieWipe/SbieWipe.exe',
            'bin_loader/SbiePatch.sys', 'bin_loader/lua53-64.dll',
            'Certificate.dat',
        ]) {
            if (exists(f)) {
                remove(f);
                console.log(`  删除 ${f}`);
            }
        }
    }

    checkToolchain();
    buildHelpers();
    prepareBinaries();
    buildSbieWipe();
    const replaceLine = signBinaries();
    generatePatchSource(replaceLine);
    buildDriver();

    console.log('\nAll done');
}

try {
    main();
} catch (e) {
    console.error(`\nError: ${e.message}`);
    process.exit(1);
}
