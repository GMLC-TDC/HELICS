function v = force_iteration()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230866);
  end
  v = vInitialized;
end
