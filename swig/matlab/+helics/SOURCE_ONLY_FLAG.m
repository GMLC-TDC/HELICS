function v = SOURCE_ONLY_FLAG()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 13);
  end
  v = vInitialized;
end
