<script setup>
import { nextTick, ref, watch } from 'vue'
import { useRouter } from 'vue-router'
import { uploadImage } from '../services/api'

const router = useRouter()
const fileRef = ref(null)
const records = ref([])
const loading = ref(false)
const pickedName = ref('')
const listEl = ref(null)

function push(role, text, image = '') {
  records.value.push({ role, text, image, id: Date.now() + Math.random() })
}

async function scrollEnd() {
  await nextTick()
  const el = listEl.value
  if (el) el.scrollTop = el.scrollHeight
}

watch(
  () => records.value.length,
  () => scrollEnd()
)

async function submit() {
  const file = fileRef.value?.files?.[0]
  if (!file || loading.value) return

  const reader = new FileReader()
  loading.value = true

  reader.onload = async (event) => {
    const base64Url = event.target?.result
    const base64Data = String(base64Url).split(',')[1] || ''

    push('user', `已上传：${file.name}`, String(base64Url))
    await scrollEnd()

    try {
      const { response, data } = await uploadImage({ filename: file.name, image: base64Data })
      if (!response.ok) throw new Error('上传失败')
      push(
        'assistant',
        `结果：${data.class_name} · 置信度 ${Math.round((data.confidence ?? 0) * 100)}%`
      )
    } catch (err) {
      push('assistant', `失败：${err.message}`)
    } finally {
      loading.value = false
      if (fileRef.value) fileRef.value.value = ''
      pickedName.value = ''
      await scrollEnd()
    }
  }

  reader.readAsDataURL(file)
}
</script>

<template>
  <main class="upload-shell">
    <header class="upload-bar">
      <button type="button" class="btn-outline" @click="router.push('/menu')">返回</button>
      <div class="upload-title-block">
        <p class="lbl">视觉</p>
        <h1 class="upload-h1">图像识别</h1>
      </div>
      <div class="spacer" aria-hidden="true" />
    </header>

    <div ref="listEl" class="upload-scroll">
      <div v-for="item in records" :key="item.id" class="row" :class="item.role">
        <div class="card" :class="item.role">
          <span class="tag">{{ item.role === 'user' ? '我' : 'AI' }}</span>
          <p class="body">{{ item.text }}</p>
          <div v-if="item.image" class="thumb">
            <img :src="item.image" alt="预览" />
          </div>
        </div>
      </div>
      <p v-if="!records.length" class="empty">在底部选择图片后开始识别</p>
    </div>

    <form class="upload-dock" @submit.prevent="submit">
      <label class="file-row">
        <span class="file-badge">选择</span>
        <input
          ref="fileRef"
          class="file-input"
          type="file"
          accept="image/*"
          required
          @change="pickedName = fileRef?.files?.[0]?.name || ''"
        />
        <span class="file-name">{{ pickedName || '未选择文件' }}</span>
      </label>
      <button class="dock-btn" type="submit" :disabled="loading">
        {{ loading ? '识别中…' : '识别' }}
      </button>
    </form>
  </main>
</template>

<style scoped>
.upload-shell {
  --edge: rgba(255, 255, 255, 0.1);
  --muted: #9ca3af;

  height: 100vh;
  height: 100dvh;
  width: 100%;
  margin: 0;
  padding: 10px;
  box-sizing: border-box;
  display: flex;
  flex-direction: column;
  gap: 10px;
  background: #080808;
  color: #f3f4f6;
  overflow: hidden;
}

.upload-bar {
  flex-shrink: 0;
  display: grid;
  grid-template-columns: auto 1fr auto;
  align-items: center;
  gap: 16px;
  padding: 14px 20px;
  border-radius: 20px;
  border: 1px solid var(--edge);
  background: rgba(14, 14, 14, 0.95);
  box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.04);
}

.spacer {
  width: 64px;
}

.btn-outline {
  padding: 8px 14px;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: #d1d5db;
  background: transparent;
  border: 1px solid rgba(255, 255, 255, 0.14);
  border-radius: 12px;
  cursor: pointer;
  transition: border-color 0.15s ease, background 0.15s ease;
}

.btn-outline:hover {
  border-color: rgba(255, 255, 255, 0.26);
  background: rgba(255, 255, 255, 0.05);
}

.upload-title-block {
  text-align: center;
  min-width: 0;
}

.lbl {
  margin: 0;
  font-size: 0.625rem;
  font-weight: 600;
  letter-spacing: 0.14em;
  text-transform: uppercase;
  color: #6b7280;
}

.upload-h1 {
  margin: 6px 0 0;
  font-size: 1rem;
  font-weight: 600;
  letter-spacing: 0.14em;
  text-transform: uppercase;
}

.upload-scroll {
  flex: 1;
  min-height: 0;
  overflow-y: auto;
  padding: 16px 18px;
  border-radius: 20px;
  border: 1px solid var(--edge);
  background: rgba(12, 12, 12, 0.98);
  box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.04);
}

.upload-scroll::-webkit-scrollbar {
  width: 8px;
}

.upload-scroll::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.1);
  border-radius: 99px;
}

.row {
  display: flex;
  margin-bottom: 16px;
}

.row.user {
  justify-content: flex-end;
}

.row.assistant {
  justify-content: flex-start;
}

.card {
  max-width: min(88%, 520px);
  padding: 14px 17px;
  border: 1px solid var(--edge);
  border-radius: 18px;
}

.card.user {
  background: rgba(28, 28, 28, 0.95);
  border-bottom-right-radius: 6px;
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.22);
}

.card.assistant {
  background: rgba(20, 20, 20, 0.95);
  border-bottom-left-radius: 6px;
}

.tag {
  display: block;
  font-size: 0.625rem;
  font-weight: 700;
  letter-spacing: 0.12em;
  color: #6b7280;
  margin-bottom: 8px;
}

.body {
  margin: 0;
  font-size: 0.9375rem;
  line-height: 1.55;
  color: #e5e7eb;
}

.thumb {
  margin-top: 12px;
  border-radius: 14px;
  overflow: hidden;
  border: 1px solid rgba(255, 255, 255, 0.1);
  background: #0a0a0a;
  max-width: min(100%, 300px);
}

.thumb img {
  display: block;
  width: 100%;
  height: auto;
}

.empty {
  margin: 0;
  padding: 44px 16px;
  text-align: center;
  font-size: 0.875rem;
  color: var(--muted);
}

.upload-dock {
  flex-shrink: 0;
  display: grid;
  grid-template-columns: 1fr auto;
  gap: 12px;
  align-items: center;
  padding: 14px 16px;
  border-radius: 18px;
  border: 1px solid var(--edge);
  background: rgba(14, 14, 14, 0.95);
  box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.04);
}

.file-row {
  position: relative;
  display: flex;
  align-items: center;
  gap: 12px;
  min-width: 0;
  padding: 10px 14px;
  border-radius: 14px;
  border: 1px solid rgba(255, 255, 255, 0.08);
  background: rgba(0, 0, 0, 0.35);
  cursor: pointer;
  transition: border-color 0.15s ease;
}

.file-row:hover {
  border-color: rgba(255, 255, 255, 0.16);
}

.file-input {
  position: absolute;
  inset: 0;
  opacity: 0;
  cursor: pointer;
}

.file-badge {
  flex-shrink: 0;
  padding: 7px 12px;
  font-size: 0.625rem;
  font-weight: 600;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  color: #0a0a0a;
  background: #d4d4d4;
  border-radius: 999px;
  border: 1px solid #a3a3a3;
}

.file-name {
  font-size: 0.8125rem;
  color: var(--muted);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.dock-btn {
  padding: 12px 24px;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.14em;
  text-transform: uppercase;
  color: #0a0a0a;
  background: #e5e5e5;
  border: 1px solid #d4d4d4;
  border-radius: 14px;
  cursor: pointer;
  box-shadow: 0 4px 14px rgba(0, 0, 0, 0.2);
  transition: background 0.15s ease;
}

.dock-btn:hover:not(:disabled) {
  background: #fafafa;
}

.dock-btn:disabled {
  opacity: 0.4;
  cursor: not-allowed;
}

@media (max-width: 560px) {
  .upload-bar {
    grid-template-columns: 1fr;
    text-align: center;
  }

  .spacer {
    display: none;
  }

  .upload-dock {
    grid-template-columns: 1fr;
  }

  .dock-btn {
    min-height: 44px;
  }
}
</style>
