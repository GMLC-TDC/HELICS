function v = helics_other_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230863);
  end
  v = vInitialized;
end
