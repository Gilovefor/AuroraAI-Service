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

async function scrollListEnd() {
  await nextTick()
  const el = listEl.value
  if (el) el.scrollTop = el.scrollHeight
}

watch(
  () => records.value.length,
  () => scrollListEnd()
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
    await scrollListEnd()

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
      await scrollListEnd()
    }
  }

  reader.readAsDataURL(file)
}
</script>

<template>
  <main class="upload-shell">
    <header class="upload-bar">
      <button type="button" class="btn-ghost" @click="router.push('/menu')">返回</button>
      <div class="upload-bar-text">
        <p class="lbl">视觉</p>
        <h1 class="upload-title">图像识别</h1>
      </div>
      <div class="upload-bar-spacer" aria-hidden="true" />
    </header>

    <div ref="listEl" class="upload-feed">
      <div v-for="item in records" :key="item.id" class="feed-row" :class="item.role">
        <div class="card" :class="item.role">
          <span class="card-tag">{{ item.role === 'user' ? '我' : 'AI' }}</span>
          <p class="card-body">{{ item.text }}</p>
          <div v-if="item.image" class="thumb">
            <img :src="item.image" alt="预览" />
          </div>
        </div>
      </div>
      <p v-if="!records.length" class="feed-empty">在底部选择图片后开始识别</p>
    </div>

    <form class="upload-dock" @submit.prevent="submit">
      <label class="file-strip">
        <span class="file-pill">选择文件</span>
        <input
          ref="fileRef"
          class="file-native"
          type="file"
          accept="image/*"
          required
          @change="pickedName = fileRef?.files?.[0]?.name || ''"
        />
        <span class="file-name">{{ pickedName || '未选择' }}</span>
      </label>
      <button class="dock-submit" type="submit" :disabled="loading">
        {{ loading ? '识别中…' : '识别' }}
      </button>
    </form>
  </main>
</template>

<style scoped>
.upload-shell {
  --line: rgba(255, 255, 255, 0.1);
  --muted: #9ca3af;

  height: 100vh;
  height: 100dvh;
  width: 100%;
  margin: 0;
  box-sizing: border-box;
  display: flex;
  flex-direction: column;
  background: #0a0a0a;
  color: #f3f4f6;
  overflow: hidden;
}

.upload-bar {
  flex-shrink: 0;
  display: grid;
  grid-template-columns: auto 1fr auto;
  align-items: center;
  gap: 16px;
  padding: 14px 18px;
  border-bottom: 1px solid var(--line);
  background: #0c0c0c;
}

.upload-bar-spacer {
  width: 72px;
}

.btn-ghost {
  padding: 8px 12px;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: #d1d5db;
  background: transparent;
  border: 1px solid rgba(255, 255, 255, 0.14);
  cursor: pointer;
  transition: border-color 0.15s ease, background 0.15s ease;
}

.btn-ghost:hover {
  border-color: rgba(255, 255, 255, 0.28);
  background: rgba(255, 255, 255, 0.05);
}

.upload-bar-text {
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

.upload-title {
  margin: 6px 0 0;
  font-size: 0.9375rem;
  font-weight: 600;
  letter-spacing: 0.14em;
  text-transform: uppercase;
}

.upload-feed {
  flex: 1;
  min-height: 0;
  overflow-y: auto;
  padding: 18px 20px;
  background: #111;
}

.upload-feed::-webkit-scrollbar {
  width: 8px;
}

.upload-feed::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.1);
  border-radius: 99px;
}

.feed-row {
  display: flex;
  margin-bottom: 16px;
}

.feed-row.user {
  justify-content: flex-end;
}

.feed-row.assistant {
  justify-content: flex-start;
}

.card {
  max-width: min(88%, 520px);
  padding: 14px 16px;
  border: 1px solid var(--line);
}

.card.user {
  background: #1a1a1a;
  border-bottom-right-radius: 2px;
}

.card.assistant {
  background: #141414;
  border-bottom-left-radius: 2px;
}

.card-tag {
  display: block;
  font-size: 0.625rem;
  font-weight: 700;
  letter-spacing: 0.14em;
  color: #6b7280;
  margin-bottom: 8px;
}

.card-body {
  margin: 0;
  font-size: 0.9375rem;
  line-height: 1.55;
  color: #e5e7eb;
}

.thumb {
  margin-top: 12px;
  border: 1px solid rgba(255, 255, 255, 0.12);
  background: #0a0a0a;
  max-width: min(100%, 320px);
}

.thumb img {
  display: block;
  width: 100%;
  height: auto;
}

.feed-empty {
  margin: 0;
  padding: 48px 16px;
  text-align: center;
  font-size: 0.875rem;
  color: var(--muted);
}

.upload-dock {
  flex-shrink: 0;
  display: grid;
  grid-template-columns: 1fr auto;
  gap: 14px;
  align-items: center;
  padding: 14px 18px 18px;
  border-top: 1px solid var(--line);
  background: #0c0c0c;
}

.file-strip {
  position: relative;
  display: flex;
  align-items: center;
  gap: 14px;
  min-width: 0;
  padding: 10px 12px;
  border: 1px solid var(--line);
  background: rgba(0, 0, 0, 0.35);
  cursor: pointer;
}

.file-strip:hover {
  border-color: rgba(255, 255, 255, 0.2);
}

.file-native {
  position: absolute;
  inset: 0;
  opacity: 0;
  cursor: pointer;
}

.file-pill {
  flex-shrink: 0;
  padding: 6px 12px;
  font-size: 0.625rem;
  font-weight: 600;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: #0a0a0a;
  background: #d4d4d4;
  border: 1px solid #a3a3a3;
}

.file-name {
  font-size: 0.8125rem;
  color: var(--muted);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.dock-submit {
  padding: 12px 24px;
  font-size: 0.6875rem;
  font-weight: 600;
  letter-spacing: 0.16em;
  text-transform: uppercase;
  color: #0a0a0a;
  background: #e5e5e5;
  border: 1px solid #d4d4d4;
  cursor: pointer;
  transition: background 0.15s ease;
}

.dock-submit:hover:not(:disabled) {
  background: #fafafa;
}

.dock-submit:disabled {
  opacity: 0.4;
  cursor: not-allowed;
}

@media (max-width: 560px) {
  .upload-bar {
    grid-template-columns: 1fr;
    text-align: center;
  }

  .upload-bar-spacer {
    display: none;
  }

  .upload-dock {
    grid-template-columns: 1fr;
  }

  .dock-submit {
    min-height: 44px;
  }
}
</style>
