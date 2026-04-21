import path from 'node:path'
import { fileURLToPath } from 'node:url'
import { defineConfig, loadEnv } from 'vite'
import vue from '@vitejs/plugin-vue'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
/** 与 `web` 同级的 `AIApps/images`（相对本配置文件，避免从各视图里硬算 `../`） */
const aiappsImagesDir = path.resolve(__dirname, '../../images')

export default defineConfig(({ mode }) => {
  const env = loadEnv(mode, process.cwd(), '')
  const apiTarget = env.VITE_API_TARGET || 'http://127.0.0.1:8080'

  const apiProxy = {
    '/api': {
      target: apiTarget,
      changeOrigin: true,
      rewrite: (p) => p.replace(/^\/api/, '')
    }
  }

  const devServer = {
    host: '0.0.0.0',
    port: 5173,
    // 5173 被占用时改用 5174…，避免进程直接退出却误以为「dev 已启动」
    strictPort: false,
    proxy: apiProxy,
    // 源码在 `web` 外（如 @aiapps-images）时，开发态需显式放行，否则可能读不到或 HMR 异常
    fs: {
      allow: [__dirname, path.resolve(__dirname, '..'), path.resolve(__dirname, '../..')]
    },
    // 减少浏览器在 dev 下强缓存 HTML / 模块导致的「改了代码刷新仍像旧版」
    headers: {
      'Cache-Control': 'no-store'
    },
    // 共享盘 / 虚拟机里监听常失效，改代码后 UI 不更新时：VITE_USE_POLLING=1 npm run dev
    ...(process.env.VITE_USE_POLLING === '1'
      ? { watch: { usePolling: true, interval: 800 } }
      : {})
  }

  return {
    plugins: [vue()],
    resolve: {
      alias: {
        '@aiapps-images': aiappsImagesDir
      }
    },
    build: {
      outDir: 'dist',
      emptyOutDir: true,
      rollupOptions: {
        external: ['fsevents']
      }
    },
    optimizeDeps: {
      exclude: ['fsevents']
    },
    server: devServer,
    preview: { ...devServer }
  }
})
