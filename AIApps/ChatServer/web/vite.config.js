import path from 'node:path'
import { fileURLToPath } from 'node:url'
import { defineConfig, loadEnv } from 'vite'
import vue from '@vitejs/plugin-vue'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const aiappsImagesDir = path.resolve(__dirname, '../../images')

export default defineConfig(({ mode }) => {
  const env = loadEnv(mode, process.cwd(), '')
  const apiTarget = env.VITE_API_TARGET || 'http://127.0.0.1:8080'

  const devServer = {
    host: '0.0.0.0',
    port: 5173,
    strictPort: false,
    proxy: {
      '/api': {
        target: apiTarget,
        changeOrigin: true,
        rewrite: (p) => p.replace(/^\/api/, '')
      }
    },
    fs: {
      allow: [__dirname, path.resolve(__dirname, '..'), path.resolve(__dirname, '../..')]
    },
    headers: {
      'Cache-Control': 'no-store'
    },
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
      outDir: '../resource/dist',
      emptyOutDir: true
    },
    server: devServer,
    preview: { ...devServer }
  }
})
