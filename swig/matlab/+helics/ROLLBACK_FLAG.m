function v = ROLLBACK_FLAG()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 9);
  end
  v = vInitialized;
end
