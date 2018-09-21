function v = HELICS_CORE_TYPE_TCP()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230917);
  end
  v = vInitialized;
end
